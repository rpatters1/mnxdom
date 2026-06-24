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

#include <stdexcept>

#include "Sequence.h"
#include "music_theory/music_theory.hpp"

namespace mnx::util {

inline music_theory::NoteName toMusicTheoryNoteName(NoteStep step)
{
    switch (step) {
    case NoteStep::C: return music_theory::NoteName::C;
    case NoteStep::D: return music_theory::NoteName::D;
    case NoteStep::E: return music_theory::NoteName::E;
    case NoteStep::F: return music_theory::NoteName::F;
    case NoteStep::G: return music_theory::NoteName::G;
    case NoteStep::A: return music_theory::NoteName::A;
    case NoteStep::B: return music_theory::NoteName::B;
    }
    throw std::invalid_argument("Unknown NoteStep value.");
}

inline NoteStep toMnxNoteStep(music_theory::NoteName noteName)
{
    switch (noteName) {
    case music_theory::NoteName::C: return NoteStep::C;
    case music_theory::NoteName::D: return NoteStep::D;
    case music_theory::NoteName::E: return NoteStep::E;
    case music_theory::NoteName::F: return NoteStep::F;
    case music_theory::NoteName::G: return NoteStep::G;
    case music_theory::NoteName::A: return NoteStep::A;
    case music_theory::NoteName::B: return NoteStep::B;
    }
    throw std::invalid_argument("Unknown music_theory::NoteName value.");
}

inline music_theory::Pitch toMusicTheoryPitch(const sequence::Pitch::Required& pitch)
{
    return {
        toMusicTheoryNoteName(pitch.step),
        pitch.octave,
        pitch.alter
    };
}

inline music_theory::Pitch toMusicTheoryPitch(const sequence::Pitch& pitch)
{
    return toMusicTheoryPitch(static_cast<sequence::Pitch::Required>(pitch));
}

} // namespace mnx::util
