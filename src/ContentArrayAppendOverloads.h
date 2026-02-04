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
inline sequence::Space ContentArray::append<sequence::Space, FractionValue>(const FractionValue& duration)
{
    return appendWithType<sequence::Space>(duration);
}

template <>
inline sequence::MultiNoteTremolo ContentArray::append<sequence::MultiNoteTremolo, int, NoteValueQuantity::Required>(
    const int& numberOfMarks, const NoteValueQuantity::Required& noteValueQuant)
{
    return appendWithType<sequence::MultiNoteTremolo>(numberOfMarks, noteValueQuant);
}

template <>
inline sequence::Tuplet ContentArray::append<sequence::Tuplet, NoteValueQuantity::Required, NoteValueQuantity::Required>(
    const NoteValueQuantity::Required& innerNoteValueQuant, const NoteValueQuantity::Required& outerNoteValueQuant)
{
    return appendWithType<sequence::Tuplet>(innerNoteValueQuant, outerNoteValueQuant);
}

} // namespace mnx

#endif // DOXYGEN_SHOULD_IGNORE_THIS
