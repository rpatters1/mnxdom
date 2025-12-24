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

namespace mnx {

/**
 * @enum AccidentalEnclosureSymbol
 * @brief The types of accidental enclosure symbols
 */
enum class AccidentalEnclosureSymbol
{
    Brackets,           ///< stems down (default)
    Parenthesis         ///< stems up
};

/**
 * @enum AutoYesNo
 * @brief 3-state choice enum
 */
enum class AutoYesNo
{
    Auto,
    Yes,
    No
};

/**
 * @enum BarlineType
 * @brief The types of barlines supported.
 */
enum class BarlineType
{
    Regular,        ///< the default (normal barline)
    Dashed,         ///< dashed barline
    Dotted,         ///< dotted barline
    Double,         ///< two thin lines together
    Final,          ///< thin line followed by thick line
    Heavy,          ///< single thick line
    HeavyHeavy,     ///< two thick lines
    HeavyLight,     ///< thick ling followed by thin line
    NoBarline,      ///< no barline (barline is invisible)
    Short,          ///< length 2 spaces centered on middle staff line.
    Tick            ///< tick barline
};

/**
 * @enum  BeamHookDirection
 * @brief The types of jumps.
 */
enum class BeamHookDirection
{
    Auto,           ///< Automatic hook direction
    Left,           ///< Force hook left
    Right           ///< Force hook right
};

/**
 * @enum BreathMarkSymbol
 * @brief The symbols available for a breath mark
 */
enum class BreathMarkSymbol
{
    Comma,   ///< comma symbol
    Tick,    ///< tick symbol
    Upbow,   ///< upbow symbol
    Salzedo  ///< salzedo symbol
};

/**
 * @enum ClefSign
 * @brief The values available in a clef sign object
 */
enum class ClefSign
{
    CClef,          ///< C Clef
    FClef,          ///< F Clef
    GClef           ///< G Clef
};

/**
 * @enum GraceType
 * @brief Options for how to perform grace notes.
 */
enum class GraceType
{
    StealPrevious,      ///< steal time from the previous non-grace.
    StealFollowing,     ///< steak time from the following non-grace.
    MakeTime            ///< add extra time for the grace note.
};

/**
 * @enum JumpType
 * @brief The types of jumps.
 */
enum class JumpType
{
    DsAlFine,       ///< D.S. al Fine
    Segno           ///< Dal Segno
};

/**
 * @enum LabelRef
 * @brief The values available in a labelref
 */
enum class LabelRef
{
    Name,           ///< the full name from the part (the default)
    ShortName       ///< the abbreviated name from the part
};

/**
 * @enum LayoutSymbol
 * @brief The symbols available to bracket a staff group
 */
enum class LayoutSymbol
{
    NoSymbol,       ///< the default (none)
    Brace,          ///< piano brace
    Bracket         ///< bracket
};

/**
 * @enum LineType
 * @brief Specifies the visual style of a line in (specifically) slurs
 */
enum class LineType
{
    Solid,   ///< A solid (unbroken) line
    Dashed,  ///< A dashed line
    Dotted,  ///< A dotted line
    Wavy     ///< A wavy line
};

/**
 * @enum LyricLineType
 * @brief The symbols available to bracket a staff group
 */
enum class LyricLineType
{
    End,            ///< the end of a word
    Middle,         ///< the middle of a word
    Start,          ///< the start of a word
    Whole           ///< this lyric is a whole word
};

/**
 * @enum MarkingUpDown
 * @brief The values available in an event marking "pointing" property
 */
enum class MarkingUpDown
{
    Down,           ///< down (default)
    Up              ///< up
};

/**
 * @enum NoteStep
 * @brief The diatonic note step values.
 *
 * These are maintained in order from [C..B] == [0..6] to allow correct math on them.
 */
enum class NoteStep : int
{
    C = 0,
    D = 1,
    E = 2,
    F = 3,
    G = 4,
    A = 5,
    B = 6
};

/**
 * @enum NoteValueBase
 * @brief The note values allowed in MNX
 */
enum class NoteValueBase
{
    Note4096th,
    Note2048th,
    Note1024th,
    Note512th,
    Note256th,
    Note128th,
    Note64th,
    Note32nd,
    Note16th,
    Eighth,
    Quarter,
    Half,
    Whole,
    Breve,
    Longa,
    Maxima,
    DuplexMaxima
};

/**
 * @enum OttavaAmount
 * @brief Valid values for ottava amount
 */
enum class OttavaAmount : int
{
    OctaveDown = -1,            ///< 8vb
    TwoOctavesDown = -2,        ///< 15mb
    ThreeOctavesDown = -3,      ///< 22mb
    OctaveUp = 1,               ///< 8va
    TwoOctavesUp = 2,           ///< 15ma
    ThreeOctavesUp = 3          ///< 22ma
};

/**
 * @enum OttavaAmountOrZero
 * @brief Valid values for octave displacment amount, including zero for no transposition.
 *
 * These values represent transposition intervals in octaves. Negative values
 * indicate downward transposition, positive values indicate upward transposition,
 * and zero indicates no transposition. This type may be used for clefs or other
 * elements where octave displacement is relevant.
 */
enum class OttavaAmountOrZero : int
{
    NoTransposition     = 0,   ///< No transposition
    ThreeOctavesDown    = -3,  ///< Transpose down three octaves
    TwoOctavesDown      = -2,  ///< Transpose down two octaves
    OctaveDown          = -1,  ///< Transpose down one octave
    OctaveUp            = 1,   ///< Transpose up one octave
    TwoOctavesUp        = 2,   ///< Transpose up two octaves
    ThreeOctavesUp      = 3    ///< Transpose up three octaves
};

/**
 * @enum SlurTieEndLocation
 * @brief Specifies the location of the end of a slur or tie, if it doesn't have a target
 */
enum class SlurTieEndLocation
{
    Outgoing,
    Incoming,
};

/**
 * @enum SlurTieSide
 * @brief Specifies the direction of a slur or tie.
 */
enum class SlurTieSide
{
    Down,   ///< curved down, the tips face up
    Up,     ///< curved up, the tips face down
};

/**
 * @enum StemDirection
 * @brief The values available in a labelref
 */
enum class StemDirection
{
    Down,           ///< stems down (default)
    Up              ///< stems up
};

/**
 * @enum TimeSignatureUnit
 * @brief Valid units for the lower numbers of time signatures
 */
enum class TimeSignatureUnit : int
{
    Whole = 1,
    Half = 2,
    Quarter = 4,
    Eighth = 8,
    Value16th = 16,
    Value32nd = 32,
    Value64th = 64,
    Value128th = 128
};

/**
 * @enum TupletDisplaySetting
 * @brief Controls display of a tuplet's number or note value
 */
enum class TupletDisplaySetting
{
    Inner,
    NoNumber,
    Both
};

} // namespace mnx

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

namespace nlohmann {

template<>
struct adl_serializer<mnx::TimeSignatureUnit>
{
    template<typename BasicJsonType>
    static mnx::TimeSignatureUnit from_json(const BasicJsonType& j)
    { return mnx::TimeSignatureUnit(j.template get<int>()); }

    template<typename BasicJsonType>
    static void to_json(BasicJsonType& j, const mnx::TimeSignatureUnit& value)
    { j = int(value); }
};

template<>
struct adl_serializer<mnx::OttavaAmount>
{
    template<typename BasicJsonType>
    static mnx::OttavaAmount from_json(const BasicJsonType& j)
    { return mnx::OttavaAmount(j.template get<int>()); }

    template<typename BasicJsonType>
    static void to_json(BasicJsonType& j, const mnx::OttavaAmount& value)
    { j = int(value); }
};

template<>
struct adl_serializer<mnx::OttavaAmountOrZero>
{
    template<typename BasicJsonType>
    static mnx::OttavaAmountOrZero from_json(const BasicJsonType& j)
    { return mnx::OttavaAmountOrZero(j.template get<int>()); }

    template<typename BasicJsonType>
    static void to_json(BasicJsonType& j, const mnx::OttavaAmountOrZero& value)
    { j = int(value); }
};

} // namespace nlohmann

#endif // DOXYGEN_SHOULD_IGNORE_THIS
