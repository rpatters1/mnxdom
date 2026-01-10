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
#include <utility>

#include "mnxdom.h"
#include "music_theory/music_theory.hpp"

namespace mnx {
    
// ****************
// ***** Base *****
// ****************

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

template <typename T, typename Scope>
struct EnclosingKey {
    static_assert(sizeof(T) == 0, "EnclosingKey<T> must be specialized.");
};

template <>
struct EnclosingKey<Part, scope::Default> {
    static constexpr std::array<std::string_view, 1> value = { "parts" };
};
template std::optional<Part> Base::getEnclosingElement<Part>() const;

template <>
struct EnclosingKey<part::Measure, scope::Default> {
    static constexpr std::array<std::string_view, 3> value = { "parts", "*", "measures" };
};
template std::optional<part::Measure> Base::getEnclosingElement<part::Measure>() const;

template <>
struct EnclosingKey<Sequence, scope::Default> {
    static constexpr std::array<std::string_view, 5> value = { "parts", "*", "measures", "*", "sequences" };
};
template std::optional<Sequence> Base::getEnclosingElement<Sequence>() const;

template <>
struct EnclosingKey<ContentObject, scope::SequenceContent> {
    static constexpr std::array<std::string_view, 7> value = { "parts", "*", "measures", "*", "sequences", "*", "content" };
};
template std::optional<ContentObject> Base::getEnclosingElement<ContentObject, scope::SequenceContent>() const;

#endif // DOXYGEN_SHOULD_IGNORE_THIS

template <typename T, typename Scope>
std::optional<T> Base::getEnclosingElement() const
{
    // Compile-time pattern like {"parts"}, {"parts","*","measures"}, etc.
    constexpr auto& key = EnclosingKey<T, Scope>::value;
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

void Document::buildEntityMap(const std::optional<ErrorHandler>& errorHandler,
                              EntityMapPolicies policies)
{
    m_entityMapping.reset();
    m_entityMapping = std::make_shared<util::EntityMap>(root(), errorHandler);
    struct Position
    {
        int measureIndex{};
        FractionValue beat{};
        std::optional<unsigned> graceIndex;
    };
    auto compareGrace = [](const std::optional<unsigned>& lhs,
                           const std::optional<unsigned>& rhs,
                           bool rhsIncludesTrailingMeasureGrace) -> int {
        if (rhsIncludesTrailingMeasureGrace && rhs && rhs.value() == 0 && lhs && lhs.value() > 0) {
            return 0;
        }
        if (!lhs && !rhs) {
            return 0;
        }
        if (!lhs) {
            return -1;
        }
        if (!rhs) {
            return 1;
        }
        if (*lhs == *rhs) {
            return 0;
        }
        return *lhs < *rhs ? -1 : 1;
    };
    auto comparePosition = [&](const Position& lhs,
                               const Position& rhs,
                               bool rhsIncludesTrailingGrace) -> int {
        if (lhs.measureIndex != rhs.measureIndex) {
            return lhs.measureIndex < rhs.measureIndex ? -1 : 1;
        }
        if (lhs.beat != rhs.beat) {
            return lhs.beat < rhs.beat ? -1 : 1;
        }
        return compareGrace(lhs.graceIndex, rhs.graceIndex, rhsIncludesTrailingGrace);
    };
    auto adaptGraceIndex = [&](std::optional<unsigned> value) -> std::optional<unsigned> {
        if (!policies.ottavasRespectGraceTargets) {
            return std::nullopt;
        }
        return value;
    };
    auto adaptVoiceTarget = [&](const std::optional<std::string>& voice) -> std::optional<std::string> {
        if (!policies.ottavasRespectVoiceTargets) {
            return std::nullopt;
        }
        return voice;
    };
    // global measures
    const auto globalMeasures = global().measures();
    std::vector<FractionValue> measureDurations(globalMeasures.size(), FractionValue(1, 1));
    int measureId = 0;
    for (const auto globalMeasure : globalMeasures) {
        measureId = globalMeasure.index_or(measureId + 1);
        m_entityMapping->add(measureId, globalMeasure);
        FractionValue duration(1, 1);
        if (const auto time = globalMeasure.calcCurrentTime()) {
            duration = static_cast<FractionValue>(*time);
        }
        measureDurations[globalMeasure.calcArrayIndex()] = duration;
    }
    // parts, events, notes
    for (const auto part : parts()) {
        if (part.id()) {
            m_entityMapping->add(part.id().value(), part);
        }
        struct OttavaSpan
        {
            int staff{ 1 };
            std::optional<std::string> voice;
            int startMeasure{};
            FractionValue startBeat{};
            std::optional<unsigned> startGraceIndex;
            int endMeasure{};
            FractionValue endBeat{};
            std::optional<unsigned> endGraceIndex;
            int value{};
            bool endsAtMeasureEnd{ false };
        };
        std::vector<OttavaSpan> ottavaSpans;
        const auto calcOttavaShift = [&](int staff,
                                         const std::optional<std::string>& voice,
                                         const Position& position) {
            int total = 0;
            for (const auto& span : ottavaSpans) {
                if (staff != span.staff) {
                    continue;
                }
                if (span.voice && (!voice || *voice != *span.voice)) {
                    continue;
                }
                Position start{ span.startMeasure, span.startBeat, span.startGraceIndex };
                if (comparePosition(position, start, false) < 0) {
                    continue;
                }
                Position end{ span.endMeasure, span.endBeat, span.endGraceIndex };
                if (comparePosition(position, end, span.endsAtMeasureEnd) > 0) {
                    continue;
                }
                total += span.value;
            }
            return total;
        };
        if (const auto measures = part.measures()) {
            for (const auto measure : measures.value()) {
                const int measureIndex = static_cast<int>(measure.calcArrayIndex());
                if (const auto& ottavas = measure.ottavas()) {
                    for (const auto& ottava : ottavas.value()) {
                        OttavaSpan span;
                        span.staff = ottava.staff();
                        span.voice = adaptVoiceTarget(ottava.voice());
                        span.startMeasure = measureIndex;
                        span.startBeat = ottava.position().fraction();
                        span.startGraceIndex = adaptGraceIndex(ottava.position().graceIndex());
                        const auto endMeasure = m_entityMapping->get<mnx::global::Measure>(ottava.end().measure(), ottava);
                        span.endMeasure = static_cast<int>(endMeasure.calcArrayIndex());
                        span.endBeat = ottava.end().position().fraction();
                        span.endGraceIndex = adaptGraceIndex(ottava.end().position().graceIndex());
                        span.value = static_cast<int>(ottava.value());
                        if (span.endMeasure >= 0) {
                            const auto measureIdx = static_cast<size_t>(span.endMeasure);
                            if (measureIdx < measureDurations.size()) {
                                span.endsAtMeasureEnd = span.endBeat == measureDurations[measureIdx];
                            }
                        }
                        ottavaSpans.push_back(std::move(span));
                    }
                }
                for (const auto sequence : measure.sequences()) {
                    struct PendingGraceEvent
                    {
                        sequence::Event event;
                        FractionValue startTime;
                        int staff;
                        std::optional<std::string> voice;
                    };

                    std::vector<PendingGraceEvent> pendingGraceEvents;
                    const auto sequenceVoice = sequence.voice();
                    const int sequenceStaff = sequence.staff();

                    auto storeOttavaShift = [&](const sequence::Event& event,
                                                const FractionValue& start,
                                                std::optional<unsigned> graceIndex,
                                                int staffNumber,
                                                const std::optional<std::string>& voiceLabel) {
                        Position position{ measureIndex, start, adaptGraceIndex(graceIndex) };
                        const int shift = calcOttavaShift(staffNumber, voiceLabel, position);
                        m_entityMapping->setEventOttavaShift(event.pointer().to_string(), shift);
                    };

                    auto flushPendingGraceEvents = [&]() {
                        if (pendingGraceEvents.empty()) {
                            return;
                        }
                        unsigned graceIndex = 1;
                        for (auto it = pendingGraceEvents.rbegin(); it != pendingGraceEvents.rend(); ++it) {
                            storeOttavaShift(it->event,
                                             it->startTime,
                                             std::optional<unsigned>(graceIndex),
                                             it->staff,
                                             it->voice);
                            ++graceIndex;
                        }
                        pendingGraceEvents.clear();
                    };

                    util::SequenceWalkHooks hooks;
                    hooks.onEvent = [&](const sequence::Event& event,
                                        const FractionValue& startTime,
                                        const FractionValue&,
                                        util::SequenceWalkContext& ctx) -> bool {
                        if (event.id()) {
                            m_entityMapping->add(event.id().value(), event);
                        }
                        if (auto notes = event.notes()) {
                            for (const auto note : notes.value()) {
                                if (note.id()) {
                                    m_entityMapping->add(note.id().value(), note);
                                }
                            }
                        }
                        if (auto kitNotes = event.kitNotes()) {
                            for (const auto kitNote : kitNotes.value()) {
                                if (kitNote.id()) {
                                    m_entityMapping->add(kitNote.id().value(), kitNote);
                                }
                            }
                        }
                        const int eventStaff = event.staff().value_or(sequenceStaff);
                        if (ctx.inGrace) {
                            pendingGraceEvents.push_back(PendingGraceEvent{
                                event,
                                startTime,
                                eventStaff,
                                sequenceVoice
                            });
                            return true;
                        }
                        flushPendingGraceEvents();
                        storeOttavaShift(event,
                                         startTime,
                                         std::optional<unsigned>(0),
                                         eventStaff,
                                         sequenceVoice);
                        return true;
                    };

                    const bool walked = util::walkSequenceContent(sequence, hooks);
                    MNX_ASSERT_IF(!walked) {
                        throw std::logic_error("Sequence walk aborted unexpectedly while building ID mapping.");
                    }
                    flushPendingGraceEvents();
                }
                if (const auto& beams = measure.beams()) {
                    auto walkBeamLevels = [&](const part::Beam& beam,
                                              int level,
                                              const auto& selfRef) -> void {
                        const auto events = beam.events();
                        if (!events.empty()) {
                            m_entityMapping->setEventBeamStartLevel(events[0], level);
                        }
                        if (const auto childBeams = beam.beams()) {
                            for (const auto& child : childBeams.value()) {
                                selfRef(child, level + 1, selfRef);
                            }
                        }
                    };
                    for (const auto& beam : beams.value()) {
                        for (const auto& eventId : beam.events()) {
                            m_entityMapping->addEventToBeam(eventId, beam);
                        }
                        walkBeamLevels(beam, 1, walkBeamLevels);
                    }
                }
            }
        }
    }
    // layouts
    if (const auto layoutArray = layouts()) {
        for (const auto layout : layoutArray.value()) {
            if (layout.id().has_value()) {
                m_entityMapping->add(layout.id().value(), layout);
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

    return FractionValue(num, den).reduced();
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
    while (!next.time()) {
        if (currentIndex == 0) {
            return std::nullopt;
        }
        next = measures[--currentIndex];
    }
    return next.time();
}

std::optional<KeySignature> global::Measure::calcCurrentKey() const
{
    auto measures = parent<Array<global::Measure>>();
    global::Measure next = *this;
    size_t currentIndex = next.calcArrayIndex();
    while (!next.key()) {
        if (currentIndex == 0) {
            return std::nullopt;
        }
        next = measures[--currentIndex];
    }
    return next.key();
}

KeySignature::Fields global::Measure::calcCurrentKeyFields() const
{
    if (auto currentKey = calcCurrentKey()) {
        return currentKey.value();
    }
    return 0;
}

// *************************
// ***** part::Measure *****
// *************************

global::Measure part::Measure::getGlobalMeasure() const
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

KeySignature::Fields part::PartTransposition::calcTransposedKey(const KeySignature::Fields& concertKey) const
{
    const auto i = interval();
    int alteration = music_theory::calcAlterationFrom12EdoHalfsteps(i.staffDistance(), i.halfSteps());
    int result = concertKey.fifths + music_theory::calcKeySigChangeFromInterval(i.staffDistance(), alteration);
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
    auto container = this->container<ContentObject>();
    return container.type() == sequence::Grace::ContentTypeValue;
}

bool sequence::Event::isTremolo() const
{
    // Note that the top level sequence container is not a ContentObject,
    // but it does not matter for the purposes of this function. The type()
    // function returns a value other than "tremolo" in that case, which is all
    // that matters here.
    auto container = this->container<ContentObject>();
    return container.type() == sequence::MultiNoteTremolo::ContentTypeValue;
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
    auto result = getEnclosingElement<Sequence>();
    MNX_ASSERT_IF(!result.has_value()) {
        throw std::logic_error("Event \"" + id_or("<no-id>") + "\" at \"" + pointer().to_string() + "\" has no top-level sequence index.");
    }
    return result.value().calcArrayIndex();
}

FractionValue sequence::Event::calcDuration() const
{
    if (measure()) {
        auto partMeasure = getEnclosingElement<part::Measure>();
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
    auto sequence = getEnclosingElement<Sequence>();
    MNX_ASSERT_IF(!sequence) {
        throw std::logic_error("Event \"" + id_or("<no-id>") + "\" at \"" + pointer().to_string() + "\" is not contained in a sequence.");
    }

    auto retval = std::optional<FractionValue>();
    auto thisPtr = pointer();
    util::iterateSequenceEvents(sequence.value(), [&](const sequence::Event& event, 
                                                      const FractionValue& startDuration,
                                                      const FractionValue& /*actualDuration*/) -> bool {
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

bool sequence::Pitch::isSamePitch(const Pitch::Fields& src) const
{
    if (src.alter == alter()
        && src.octave == octave()
        && src.step == step()) {
        return true;
    }
    music_theory::Transposer t(music_theory::calcDisplacement(int(src.step), src.octave), src.alter);
    return t.isEnharmonicEquivalent(music_theory::calcDisplacement(int(step()), octave()), alter());
}

sequence::Pitch::Fields sequence::Pitch::calcTransposed() const
{
    auto sequence = getEnclosingElement<Sequence>();
    MNX_ASSERT_IF(!sequence) {
        throw std::logic_error("unable to find enclosing sequence for pitch.");
    }
    auto partMeasure = sequence->container<part::Measure>();
    auto globalMeasure = partMeasure.getGlobalMeasure();
    auto part = partMeasure.getEnclosingElement<Part>();
    MNX_ASSERT_IF(!part) {
        throw std::logic_error("unable to find enclosing part for pitch.");
    }

    if (auto partTrans = part->transposition()) {
        music_theory::Transposer t(music_theory::calcDisplacement(int(step()), octave()), alter());
        const auto interval = partTrans->interval();
        const int intervalDisp = interval.staffDistance();
        const int intervalAlt = music_theory::calcAlterationFrom12EdoHalfsteps(intervalDisp, interval.halfSteps());
        t.chromaticTranspose(intervalDisp, intervalAlt);
        int expectedKeyFifths = music_theory::calcKeySigChangeFromInterval(intervalDisp, intervalAlt)
                                    + globalMeasure.calcCurrentKeyFields().fifths;
        auto actualTransKey = partTrans->calcTransposedKey(globalMeasure.calcCurrentKeyFields());
        const int fifthsDiff = expectedKeyFifths - actualTransKey.fifths;
        const int wraps = fifthsDiff / 12;          // trunc toward zero
        if (wraps != 0) {
            t.enharmonicTranspose(wraps);
        }
        auto note = parent<sequence::Note>();
        if (auto written = note.written()) {
            t.enharmonicTranspose(written->diatonicDelta());
        }
        int newAlter = t.alteration();
        int newOctaves{};
        int newNoteType = music_theory::positiveModulus(t.displacement(), music_theory::STANDARD_DIATONIC_STEPS, &newOctaves);
        return { NoteStep(newNoteType), newOctaves + 4, newAlter };
    }
    return *this;
}

} // namespace mnx
