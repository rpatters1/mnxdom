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
#pragma once

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

namespace mnx {

template <>
template <>
inline sequence::Event ContentArray<sequence::SequenceContentObject>::append<sequence::Event, NoteValueBase>(const NoteValueBase& base)
{
    return appendWithType<sequence::Event>(base);
}

template <>
template <>
inline sequence::Event ContentArray<sequence::SequenceContentObject>::append<sequence::Event, NoteValueBase, unsigned>(
    const NoteValueBase& base, const unsigned& dots)
{
    return appendWithType<sequence::Event>(base, dots);
}

template <>
template <>
inline sequence::Space ContentArray<sequence::SequenceContentObject>::append<sequence::Space, FractionValue>(const FractionValue& duration)
{
    return appendWithType<sequence::Space>(duration);
}

template <>
template <>
inline sequence::MultiNoteTremolo ContentArray<sequence::SequenceContentObject>::append<sequence::MultiNoteTremolo, int, NoteValueQuantity::Required>(
    const int& numberOfMarks, const NoteValueQuantity::Required& noteValueQuant)
{
    return appendWithType<sequence::MultiNoteTremolo>(numberOfMarks, noteValueQuant);
}

template <>
template <>
inline sequence::Tuplet ContentArray<sequence::SequenceContentObject>::append<sequence::Tuplet, NoteValueQuantity::Required, NoteValueQuantity::Required>(
    const NoteValueQuantity::Required& innerNoteValueQuant, const NoteValueQuantity::Required& outerNoteValueQuant)
{
    return appendWithType<sequence::Tuplet>(innerNoteValueQuant, outerNoteValueQuant);
}

template <>
template <>
inline part::DynamicAccent ContentArray<part::DynamicGroup>::append<part::DynamicAccent, DynamicValue, FractionValue>(
    const DynamicValue& value, const FractionValue& position)
{
    return appendWithType<part::DynamicAccent>(value, position);
}

template <>
template <>
inline part::DynamicGradual ContentArray<part::DynamicGroup>::append<part::DynamicGradual, DynamicWedgeType, FractionValue, MeasureRhythmicPosition::Required>(
    const DynamicWedgeType& wedgeType, const FractionValue& position, const MeasureRhythmicPosition::Required& endPosition)
{
    return appendWithType<part::DynamicGradual>(wedgeType, position, endPosition);
}

template <>
template <>
inline part::DynamicImmediate ContentArray<part::DynamicGroup>::append<part::DynamicImmediate, DynamicValue, FractionValue>(
    const DynamicValue& value, const FractionValue& position)
{
    return appendWithType<part::DynamicImmediate>(value, position);
}

template <>
template <>
inline part::DynamicRelative ContentArray<part::DynamicGroup>::append<part::DynamicRelative, DynamicRelativeValue, FractionValue>(
    const DynamicRelativeValue& relativeValue, const FractionValue& position)
{
    return appendWithType<part::DynamicRelative>(relativeValue, position);
}

} // namespace mnx

#endif // DOXYGEN_SHOULD_IGNORE_THIS
