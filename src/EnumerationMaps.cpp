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
#include <string>
#include <unordered_map>

#include "mnxdom.h"

#define MNX_ENUM_MAPPING(EnumType, ...)                                       \
template <>                                                                   \
const std::unordered_map<std::string, EnumType>                               \
EnumStringMapping<EnumType>::stringToEnum() {                                 \
    static const std::unordered_map<std::string, EnumType> map = __VA_ARGS__;  \
    return map;                                                               \
}                                                                             \
static_assert(true, "") // forces semicolon after macro invocation

namespace mnx {

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

MNX_ENUM_MAPPING(AccidentalEnclosureSymbol, {
    { "brackets",       AccidentalEnclosureSymbol:: Brackets },
    { "parentheses",    AccidentalEnclosureSymbol::Parenthesis }
});

MNX_ENUM_MAPPING(AutoYesNo, {
    { "auto", AutoYesNo::Auto },
    { "yes",  AutoYesNo::Yes },
    { "no",   AutoYesNo::No }
});

MNX_ENUM_MAPPING(BarlineType, {
    { "regular",     BarlineType::Regular },
    { "dashed",      BarlineType::Dashed },
    { "dotted",      BarlineType::Dotted },
    { "double",      BarlineType::Double },
    { "final",       BarlineType::Final },
    { "heavy",       BarlineType::Heavy },
    { "heavyHeavy",  BarlineType::HeavyHeavy },
    { "heavyLight",  BarlineType::HeavyLight },
    { "noBarline",   BarlineType::NoBarline },
    { "short",       BarlineType::Short },
    { "tick",        BarlineType::Tick }
});

MNX_ENUM_MAPPING(BeamHookDirection, {
    { "left",       BeamHookDirection::Left },
    { "right",      BeamHookDirection::Right }
});

MNX_ENUM_MAPPING(BreathMarkSymbol, {
    { "comma",      BreathMarkSymbol::Comma },
    { "tick",       BreathMarkSymbol::Tick },
    { "upbow",      BreathMarkSymbol::Upbow },
    { "salzedo",    BreathMarkSymbol::Salzedo }
});

MNX_ENUM_MAPPING(ClefSign, {
    { "C",          ClefSign::CClef },
    { "F",          ClefSign::FClef },
    { "G",          ClefSign::GClef }
});

MNX_ENUM_MAPPING(JumpType, {
    { "dsalfine",   JumpType::DsAlFine },
    { "segno",      JumpType::Segno }
});

MNX_ENUM_MAPPING(LabelRef, {
    { "name",       LabelRef::Name },
    { "shortName",  LabelRef::ShortName }
});

MNX_ENUM_MAPPING(LayoutSymbol, {
    { "noSymbol",   LayoutSymbol::NoSymbol },
    { "brace",      LayoutSymbol::Brace },
    { "bracket",    LayoutSymbol::Bracket }
});

MNX_ENUM_MAPPING(LineType, {
    { "solid",      LineType::Solid },
    { "dashed",     LineType::Dashed },
    { "dotted",     LineType::Dotted },
    { "wavy",       LineType::Wavy }
});

MNX_ENUM_MAPPING(LyricLineType, {
    { "end",        LyricLineType::End },
    { "middle",     LyricLineType::Middle },
    { "start",      LyricLineType::Start },
    { "whole",      LyricLineType::Whole }
});

MNX_ENUM_MAPPING(MarkingUpDown, {
    { "down",       MarkingUpDown::Down },
    { "up",         MarkingUpDown::Up }
});

MNX_ENUM_MAPPING(NoteStep, {
    { "C", NoteStep::C },
    { "D", NoteStep::D },
    { "E", NoteStep::E },
    { "F", NoteStep::F },
    { "G", NoteStep::G },
    { "A", NoteStep::A },
    { "B", NoteStep::B }
});

MNX_ENUM_MAPPING(NoteValueBase, {
    { "4096th",       NoteValueBase::Note4096th },
    { "2048th",       NoteValueBase::Note2048th },
    { "1024th",       NoteValueBase::Note1024th },
    { "512th",        NoteValueBase::Note512th },
    { "256th",        NoteValueBase::Note256th },
    { "128th",        NoteValueBase::Note128th },
    { "64th",         NoteValueBase::Note64th },
    { "32nd",         NoteValueBase::Note32nd },
    { "16th",         NoteValueBase::Note16th },
    { "eighth",       NoteValueBase::Eighth },
    { "quarter",      NoteValueBase::Quarter },
    { "half",         NoteValueBase::Half },
    { "whole",        NoteValueBase::Whole },
    { "breve",        NoteValueBase::Breve },
    { "longa",        NoteValueBase::Longa },
    { "maxima",       NoteValueBase::Maxima },
    { "duplexMaxima", NoteValueBase::DuplexMaxima }
});

MNX_ENUM_MAPPING(StemDirection, {
    { "down",       StemDirection::Down },
    { "up",         StemDirection::Up }
});

MNX_ENUM_MAPPING(SlurTieEndLocation, {
    { "outgoing",   SlurTieEndLocation::Outgoing },
    { "incoming",   SlurTieEndLocation::Incoming }
});

MNX_ENUM_MAPPING(SlurTieSide, {
    { "down",       SlurTieSide::Down },
    { "up",         SlurTieSide::Up }
});

MNX_ENUM_MAPPING(TupletDisplaySetting, {
    { "inner",      TupletDisplaySetting::Inner },
    { "noNumber",   TupletDisplaySetting::NoNumber },
    { "both",       TupletDisplaySetting::Both }
});

#endif // DOXYGEN_SHOULD_IGNORE_THIS

} // namespace mnx
