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
#include "mnxdom.h"
#include "music_theory/music_theory.hpp"

namespace mnx {
    
// ****************
// ***** Base *****
// ****************

template <typename T>
struct EnclosingKey
{
    static_assert(sizeof(T) == 0, "EnclosingKey<T> must be specialized.");
};

template <>
struct EnclosingKey<mnx::Part> {
    static constexpr std::string_view value = "/parts/"; // prefix
};
template std::optional<mnx::Part> Base::getEnclosingElement<mnx::Part>() const;

template <>
struct EnclosingKey<mnx::Sequence> {
    static constexpr std::string_view value = "sequences/"; // just a key
};
template std::optional<mnx::Sequence> Base::getEnclosingElement<mnx::Sequence>() const;

template <typename T>
std::optional<T> Base::getEnclosingElement() const
{
    constexpr std::string_view key = EnclosingKey<T>::value;
    std::string s = m_pointer.to_string();

    std::size_t pos = s.find(key);
    if (pos == std::string::npos)
        return std::nullopt;

    // Advance past the key to get the start of the index
    std::size_t after = s.find('/', pos + key.length());
    if (after == std::string::npos) {
        return T(m_root, json_pointer(s));  // key was last element
    }
    return T(m_root, json_pointer(s.substr(0, after)));
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
                            } else if (content.type() == sequence::Tuplet::ContentTypeValue) {
                                const auto tuplet = content.get<sequence::Tuplet>();
                                self(tuplet.content(), self);
                            } else if (content.type() == sequence::Grace::ContentTypeValue) {
                                const auto grace = content.get<sequence::Grace>();
                                self(grace.content(), self);
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
            m_idMapping->add(layout.id(), layout);
        }
    }
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
    auto container = this->container<mnx::ContentObject>();
    return container.type() == mnx::sequence::Grace::ContentTypeValue;
}

Sequence sequence::Event::getSequence() const
{
    auto result = getEnclosingElement<Sequence>();
    MNX_ASSERT_IF(!result.has_value()) {
        throw std::logic_error("Event \"" + id().value_or("") + "\" at \"" + pointer().to_string() + "\" is not part of a sequence.");
    }
    return result.value();
}

// ***************************
// ***** sequence::Pitch *****
// ***************************

bool sequence::Pitch::isSamePitch(const Pitch& src) const
{
    if (src.alter().value_or(0) == alter().value_or(0)
        && src.octave() == octave()
        && src.step() == step()) {
        return true;
    }
    music_theory::Transposer t(music_theory::calcDisplacement(int(src.step()), src.octave()), src.alter().value_or(0));
    return t.isEnharmonicEquivalent(music_theory::calcDisplacement(int(step()), octave()), alter().value_or(0));
}

} // namespace mnx