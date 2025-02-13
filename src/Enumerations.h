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
 * @enum JumpType
 * @brief The types of jumps.
 */
enum class JumpType
{
    DsAlFine,       ///< D.S. al Fine
    Segno           ///< Dal Segno
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
 * @enum LabelRef
 * @brief The values available in a labelref
 */
enum class LabelRef
{
    Name,           ///< the full name from the part (the default)
    ShortName       ///< the abbreviated name from the part
};

/**
 * @enum LayoutStemDirection
 * @brief The values available in a labelref
 */
enum class LayoutStemDirection
{
    Down,           ///< stems down (default)
    Up              ///< stems up
};

} // namespace mnx

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

namespace nlohmann {

#if defined(_WIN32)

// This general adl_serializer is enabled only for enum types.
// For some reason MSC does not like the direct function defintions below.
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
#else
namespace detail {

template<typename BasicJsonType>
inline void from_json(const BasicJsonType& j, mnx::TimeSignatureUnit& value)
{ value = mnx::TimeSignatureUnit(j.template get<int>()); }

template<typename BasicJsonType>
inline void to_json(BasicJsonType& j, mnx::TimeSignatureUnit value) noexcept
{ j = int(value); }

} // namespace detail
#endif

} // namespace nlohmann

#endif // DOXYGEN_SHOULD_IGNORE_THIS
