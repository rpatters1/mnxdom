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

std::optional<Part> Base::getPart()
{
    const std::string prefix = "/parts/";
    const std::string ptrStr = m_pointer.to_string();

    if (ptrStr.rfind(prefix, 0) == 0) {  // prefix match at position 0
        const std::string rest = ptrStr.substr(prefix.size());
        const std::size_t slashPos = rest.find('/');
        const std::string indexStr = (slashPos == std::string::npos) ? rest : rest.substr(0, slashPos);

        try {
            std::size_t index = std::stoul(indexStr);
            return mnx::Part(m_root, json_pointer(prefix + indexStr));
        } catch (...) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

// ********************
// ***** Document *****
// ********************

void Document::buildIdMapping()
{
    m_idMapping.reset();
    m_idMapping = std::make_shared<util::IdMapping>(root());
    /// @todo build the mapping
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
    return prevIndex.value_or(prev.calcMeasureIndex());
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