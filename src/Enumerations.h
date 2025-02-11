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

} // namespace