/*
 * Copyright (C) 2025, Robert Patterson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <cassert>

#include "mnxdom.h"
#include "music_theory/music_theory.hpp"

namespace mnx {
    
// ****************
// ***** Base *****
// ****************

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

template <typename T>
struct EnclosingKey {
    static_assert(sizeof(T) == 0, "EnclosingKey<T> must be specialized.");
};

template <>
struct EnclosingKey<mnx::Part> {
    static constexpr std::array<std::string_view, 1> value = { "parts" };
};
template std::optional<mnx::Part> Base::getEnclosingElement<mnx::Part>() const;

template <>
struct EnclosingKey<mnx::part::Measure> {
    static constexpr std::array<std::string_view, 3> value = { "parts", "*", "measures" };
};
template std::optional<mnx::part::Measure> Base::getEnclosingElement<mnx::part::Measure>() const;

template <>
struct EnclosingKey<mnx::Sequence> {
    static constexpr std::array<std::string_view, 5> value = { "parts", "*", "measures", "*", "sequences" };
};
template std::optional<mnx::Sequence> Base::getEnclosingElement<mnx::Sequence>() const;

template <>
struct EnclosingKey<mnx::sequence::ContentObject> {
    static constexpr std::array<std::string_view, 7> value = { "parts", "*", "measures", "*", "sequences", "*", "content" };
};
template std::optional<mnx::sequence::ContentObject> Base::getEnclosingElement<mnx::sequence::ContentObject>() const;

#endif // DOXYGEN_SHOULD_IGNORE_THIS

template <typename T>
std::optional<T> Base::getEnclosingElement() const
{
    // Compile-time pattern like {"parts"}, {"parts","*","measures"}, etc.
    constexpr auto& key = EnclosingKey<T>::value;
    constexpr std::size_t keySize = key.size();

    const std::string ptrStr = m_pointer.to_string();
    if (ptrStr.empty() || ptrStr[0] != '/') {
        return std::nullopt; // not a valid JSON pointer form
    }

    std::size_t pos = 1;                 // skip leading '/'
    std::size_t tokenIndex = 0;
    std::size_t enclosingEnd = std::string::npos;

    while (pos <= ptrStr.size()) {
        // Find end of current token
        std::size_t slash = ptrStr.find('/', pos);
        if (slash == std::string::npos) {
            slash = ptrStr.size();
        }
        MNX_ASSERT_IF(slash == pos) {
            break; // empty segment; shouldn't happen in valid pointers
        }

        std::string_view token(ptrStr.data() + pos, slash - pos);

        if (tokenIndex < keySize) {
            // Compare against key prefix (with "*" wildcard)
            if (key[tokenIndex] != "*" && token != key[tokenIndex]) {
                return std::nullopt;     // not under this enclosing family
            }
        } else if (tokenIndex == keySize) {
            // This is the trailing index segment we want to include
            enclosingEnd = slash;
            break;                       // we don't care about deeper segments
        }

        ++tokenIndex;
        pos = slash + 1;
    }

    // We must have seen at least keySize + 1 segments (the trailing index)
    if (enclosingEnd == std::string::npos) {
        return std::nullopt;
    }

    // pointer to the enclosing element: "/segments[0]/.../segments[keySize]"
    return T(m_root, json_pointer(ptrStr.substr(0, enclosingEnd)));
}

Document Base::document() const
{
    return Document(root());
}

// ********************
// ***** Document *****
// ********************

void Document::buildIdMapping(const std::optional<ErrorHandler>& errorHandler)
{
    m_idMapping.reset();
    m_idMapping = std::make_shared<util::IdMapping>(root(), errorHandler);
    // global measures
    const auto globalMeasures = global().measures();
    int measureId = 0;
    for (const auto globalMeasure : global().measures()) {
        measureId = globalMeasure.index_or(measureId + 1);
        m_idMapping->add(measureId, globalMeasure);
    }
    // parts, events, notes
    for (const auto part : parts()) {
        if (part.id()) {
            m_idMapping->add(part.id().value(), part);
        }
        if (const auto measures = part.measures()) {
            for (const auto measure : measures.value()) {
                for (const auto sequence : measure.sequences()) {
                    auto processContent = [&](const ContentArray& contentArray, auto&& self) -> void {
                        for (const auto content : contentArray) {
                            if (content.type() == sequence::Event::ContentTypeValue) {
                                const auto event = content.get<sequence::Event>();
                                if (event.id()) {
                                    m_idMapping->add(event.id().value(), event);
                                }
                                if (auto notes = event.notes()) {
                                    for (const auto note : notes.value()) {
                                        if (note.id()) {
                                            m_idMapping->add(note.id().value(), note);
                                        }
                                    }
                                }
                                if (auto kitNotes = event.kitNotes()) {
                                    for (const auto kitNote : kitNotes.value()) {
                                        if (kitNote.id()) {
                                            m_idMapping->add(kitNote.id().value(), kitNote);
                                        }
                                    }
                                }
                            } else if (content.type() == sequence::Tuplet::ContentTypeValue) {
                                const auto tuplet = content.get<sequence::Tuplet>();
                                self(tuplet.content(), self);
                            } else if (content.type() == sequence::Grace::ContentTypeValue) {
                                const auto grace = content.get<sequence::Grace>();
                                self(grace.content(), self);
                            } else if (content.type() == sequence::MultiNoteTremolo::ContentTypeValue) {
                                const auto tremolo = content.get<sequence::MultiNoteTremolo>();
                                self(tremolo.content(), self);
                            }
                        }
                    };
                    processContent(sequence.content(), processContent);
                }
            }
        }
    }
    // layouts
    if (const auto layoutArray = layouts()) {
        for (const auto layout : layoutArray.value()) {
            if (layout.id().has_value()) {
                m_idMapping->add(layout.id().value(), layout);
            }
        }
    }
}

std::optional<Layout> Document::findFullScoreLayout() const
{
    using StaffKey = util::StaffKey;
    using StaffKeyHash = util::StaffKeyHash;

    const auto layoutsOpt = layouts();
    if (!layoutsOpt) {
        return std::nullopt; // no layouts provided
    }

    // Build the expected (part, staff) sequence in the *part list order*.
    // Wire these to your actual part/staff APIs.
    std::vector<StaffKey> expected;

    for (auto part : parts()) {
        if (!part.id() || part.id()->empty()) {
            return std::nullopt; // parts with no ID cannot appear in any layout, no full score layout is possible.
        }
        const std::string pid = *part.id();
        const int staffCount = part.staves();
        for (int s = 1; s <= staffCount; ++s) {
            expected.push_back(StaffKey{pid, s});
        }
    }

    for (auto layout : *layoutsOpt) {
        const auto staves = util::flattenLayoutStaves(layout);
        if (!staves) {
            continue;
        }
        if (staves->size() != expected.size()) {
            continue;
        }
        // Enforce uniqueness and ordering of (part, staff) across the layout.
        std::unordered_set<StaffKey, StaffKeyHash> seen;
        seen.reserve(staves->size());

        bool ok = true;
        for (size_t i = 0; i < staves->size(); ++i) {
            const auto keys = util::analyzeLayoutStaffVoices(staves.value()[i]);
            if (!keys || keys->empty() || keys->size() > 1) {
                ok = false;
                break;
            }
            const auto& key = keys->begin();

            // Must match part/staff order exactly.
            if (key->partId != expected[i].partId || key->staffNo != expected[i].staffNo) {
                ok = false;
                break;
            }

            // Each part staff appears on exactly one layout staff.
            if (!seen.emplace(*key).second) {
                ok = false;
                break;
            }
        }

        if (ok) {
            return layout;
        }
    }

    return std::nullopt;
}

// *********************
// ***** NoteValue *****
// *********************

unsigned NoteValue::calcNumberOfFlags() const
{
    switch (base()) {
        default: return 0;
        case NoteValueBase::Eighth: return 1;
        case NoteValueBase::Note16th: return 2;
        case NoteValueBase::Note32nd: return 3;
        case NoteValueBase::Note64th: return 4;
        case NoteValueBase::Note128th: return 5;
        case NoteValueBase::Note256th: return 6;
        case NoteValueBase::Note512th: return 7;
        case NoteValueBase::Note1024th: return 8;
        case NoteValueBase::Note2048th: return 9;
        case NoteValueBase::Note4096th: return 10;
    }
}

NoteValue::operator FractionValue() const
{
    using Num = FractionValue::NumType;

    Num num = 1;
    Num den = 1;

    // Base duration, with a quarter note = 1/4
    switch (base())
    {
        case NoteValueBase::Note4096th:     den = 4096; break;
        case NoteValueBase::Note2048th:     den = 2048; break;
        case NoteValueBase::Note1024th:     den = 1024; break;
        case NoteValueBase::Note512th:      den = 512;  break;
        case NoteValueBase::Note256th:      den = 256;  break;
        case NoteValueBase::Note128th:      den = 128;  break;
        case NoteValueBase::Note64th:       den = 64;   break;
        case NoteValueBase::Note32nd:       den = 32;   break;
        case NoteValueBase::Note16th:       den = 16;   break;
        case NoteValueBase::Eighth:         den = 8;    break;
        case NoteValueBase::Quarter:        den = 4;    break;
        case NoteValueBase::Half:           den = 2;    break;
        case NoteValueBase::Whole:          den = 1;    break;
        case NoteValueBase::Breve:          num = 2;    break;  // 2/1
        case NoteValueBase::Longa:          num = 4;    break;  // 4/1
        case NoteValueBase::Maxima:         num = 8;    break;  // 8/1
        case NoteValueBase::DuplexMaxima:   num = 16;   break;  // 16/1
    }

    const unsigned numDots = dots();
    if (numDots)
    {
        // Geometric factor: 1 + 1/2 + ... + 1/2^dots
        // = (2^(dots+1) - 1) / 2^dots
        const Num factorNum = (Num(1) << (numDots + 1U)) - Num(1);
        const Num factorDen = (Num(1) << numDots);

        num *= factorNum;
        den *= factorDen;
    }

    return FractionValue(num, den);  // FractionValue will normalize internally
}

// ***************************
// ***** global::Measure *****
// ***************************

BarlineType global::Measure::calcBarlineType() const
{
    if (const auto thisBarline = barline()) {
        return thisBarline.value().type();
    }
    const auto parentArray = parent<Array<global::Measure>>();
    size_t arrayIndex = calcArrayIndex();
    return (arrayIndex + 1) == parentArray.size() ? BarlineType::Final : BarlineType::Regular;
}

int global::Measure::calcMeasureIndex() const
{
    if (auto thisIndex = index()) {
        return thisIndex.value();
    }
    size_t arrayIndex = calcArrayIndex();
    if (arrayIndex == 0) return 1;
    const auto parentArray = parent<Array<global::Measure>>();
    const auto prev = parentArray[arrayIndex - 1];
    const auto prevIndex = prev.index();
    return prevIndex.value_or(prev.calcMeasureIndex()) + 1;
}

int global::Measure::calcVisibleNumber() const
{
    return number_or(calcMeasureIndex());
}

std::optional<TimeSignature> global::Measure::calcCurrentTime() const
{
    auto measures = parent<Array<global::Measure>>();
    global::Measure next = *this;
    size_t currentIndex = next.calcArrayIndex();
    while (!next.time())
    {
        if (currentIndex == 0) {
            return std::nullopt;
        }
        next = measures[--currentIndex];
    }
    return next.time();
}

// *************************
// ***** part::Measure *****
// *************************

mnx::global::Measure part::Measure::getGlobalMeasure() const
{
    const size_t measureIndex = calcArrayIndex();
    auto globalMeasures = document().global().measures();
    MNX_ASSERT_IF (measureIndex >= globalMeasures.size()) {
        throw std::logic_error("Part measure has higher index than global measure at " + dump());
    }
    return globalMeasures[measureIndex];
}

std::optional<TimeSignature> part::Measure::calcCurrentTime() const
{
    return getGlobalMeasure().calcCurrentTime();
}

// ***********************************
// ***** part::PartTransposition *****
// ***********************************

int part::PartTransposition::calcTransposedKeyFifthsFor(const mnx::KeySignature& concertKey) const
{
    const auto i = interval();
    int alteration = music_theory::calcAlterationFrom12EdoHalfsteps(i.staffDistance(), i.halfSteps());
    int result = concertKey.fifths() + music_theory::calcKeySigChangeFromInterval(i.staffDistance(), alteration);
    if (const auto flipAt = keyFifthsFlipAt(); flipAt.has_value()) {
        constexpr int FIFTHS_WRAP = 12; // enharmonic wrap in fifths-space
        if (*flipAt >= 0) {
            // "subtract 12 fifths"
            if (result >= *flipAt) {
                result -= FIFTHS_WRAP;
            }
        } else {
            // negative flipAt means "add 12"
            if (result <= *flipAt) {
                result += FIFTHS_WRAP;
            }
        }
    }
    return result;
}

// ***************************
// ***** sequence::Event *****
// ***************************

std::optional<sequence::Note> sequence::Event::findNote(const std::string& noteId) const
{
    if (auto notes = this->notes()) {
        for (const auto note : notes.value()) {
            if (note.id() == noteId) {
                return note;
            }
        }
    }
    return std::nullopt;
}

bool sequence::Event::isGrace() const
{
    // Note that the top level sequence container is not a ContentObject,
    // but it does not matter for the purposes of this function. The type()
    // function returns a value other than "grace" in that case, which is all
    // that matters here.
    auto container = this->container<mnx::sequence::ContentObject>();
    return container.type() == mnx::sequence::Grace::ContentTypeValue;
}

bool sequence::Event::isTremolo() const
{
    // Note that the top level sequence container is not a ContentObject,
    // but it does not matter for the purposes of this function. The type()
    // function returns a value other than "tremolo" in that case, which is all
    // that matters here.
    auto container = this->container<mnx::sequence::ContentObject>();
    return container.type() == mnx::sequence::MultiNoteTremolo::ContentTypeValue;
}

Sequence sequence::Event::getSequence() const
{
    auto result = getEnclosingElement<Sequence>();
    MNX_ASSERT_IF(!result.has_value()) {
        throw std::logic_error("Event \"" + id_or("") + "\" at \"" + pointer().to_string() + "\" is not part of a sequence.");
    }
    return result.value();
}

size_t sequence::Event::getSequenceIndex() const
{
    auto result = getEnclosingElement<mnx::sequence::ContentObject>();
    MNX_ASSERT_IF(!result.has_value()) {
        throw std::logic_error("Event \"" + id_or("<no-id>") + "\" at \"" + pointer().to_string() + "\" has no top-level sequence index.");
    }
    return result.value().calcArrayIndex();
}

FractionValue sequence::Event::calcDuration() const
{
    if (measure()) {
        auto partMeasure = getEnclosingElement<mnx::part::Measure>();
        MNX_ASSERT_IF(!partMeasure) {
            throw std::logic_error("Event \"" + id_or("<no-id>") + "\" at \"" + pointer().to_string() + "\" is not contained in a part measure.");
        }
        if (auto currentTime = partMeasure.value().calcCurrentTime()) {
            return currentTime.value();
        }
    }
    if (duration().has_value()) {
        return duration().value();
    }
    return 0;
}

FractionValue sequence::Event::calcStartTime() const
{
    auto sequence = getEnclosingElement<mnx::Sequence>();
    MNX_ASSERT_IF(!sequence) {
        throw std::logic_error("Event \"" + id_or("<no-id>") + "\" at \"" + pointer().to_string() + "\" is not contained in a sequence.");
    }

    auto retval = std::optional<FractionValue>();
    auto thisPtr = pointer();
    sequence->iterateEvents([&](sequence::Event event, FractionValue startDuration, FractionValue /*actualDuration*/) -> bool {
        if (event.pointer() == thisPtr) {
            retval = startDuration;
            return false;
        }
        return true;
    });

    MNX_ASSERT_IF(!retval) {
        throw std::logic_error("Event \"" + id_or("<no-id>") + "\" at \"" + pointer().to_string() + "\" was not found in its enclosing sequence.");
    }
    return retval.value();
}

// ***************************
// ***** sequence::Pitch *****
// ***************************

bool sequence::Pitch::isSamePitch(const Pitch& src) const
{
    if (src.alter_or(0) == alter_or(0)
        && src.octave() == octave()
        && src.step() == step()) {
        return true;
    }
    music_theory::Transposer t(music_theory::calcDisplacement(int(src.step()), src.octave()), src.alter_or(0));
    return t.isEnharmonicEquivalent(music_theory::calcDisplacement(int(step()), octave()), alter_or(0));
}

// ********************
// ***** Sequence *****
// ********************

bool Sequence::iterateEvents(std::function<bool(sequence::Event event, FractionValue startDuration, FractionValue actualDuration)> iterator) const
{
    auto elapsedTime = FractionValue{0};

    auto processContent = [&](ContentArray content, FractionValue timeRatio, auto&& self) -> bool {
        for (const auto item : content) {
            if (item.type() == mnx::sequence::Event::ContentTypeValue) {
                auto event = item.get<mnx::sequence::Event>();
                auto actualDuration = event.calcDuration() * timeRatio;
                if (!iterator(event, elapsedTime, actualDuration)) {
                    return false;
                }
                elapsedTime += actualDuration;
            } else if (item.type() == mnx::sequence::Grace::ContentTypeValue) {
                auto grace = item.get<mnx::sequence::Grace>();
                if (!self(grace.content(), 0, self)) {
                    return false;
                }
            } else if (item.type() == mnx::sequence::Tuplet::ContentTypeValue) {
                auto tuplet = item.get<mnx::sequence::Tuplet>();
                if (!self(tuplet.content(), timeRatio * tuplet.ratio(), self)) {
                    return false;
                }
            } else if (item.type() == mnx::sequence::MultiNoteTremolo::ContentTypeValue) {
                auto tremolo = item.get<mnx::sequence::MultiNoteTremolo>();
                /// @todo: MNX tremolo durations.
                // Currently, inner tremolo notes are treated as time-neutral and only
                // tremolo.outer() * timeRatio advances elapsedTime. Once the MNX spec
                // clarifies per-note duration semantics for multi-note tremolos, this
                // iterator should be updated so each event gets an appropriate share of
                // the tremolo duration pie.
                if (!self(tremolo.content(), 0, self)) {
                    return false;
                }
                elapsedTime += tremolo.outer() * timeRatio;
            } else if (item.type() == mnx::sequence::Space::ContentTypeValue) {
                auto space = item.get<mnx::sequence::Space>();
                elapsedTime += space.duration() * timeRatio;
            }
        }
        return true;
    };

    return processContent(content(), 1, processContent);
}

} // namespace mnx