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

MNX_ENUM_MAPPING(LayoutSymbol, {
    { "noSymbol", LayoutSymbol::NoSymbol },
    { "brace",    LayoutSymbol::Brace },
    { "bracket",  LayoutSymbol::Bracket }
});

MNX_ENUM_MAPPING(LabelRef, {
    { "name",      LabelRef::Name },
    { "shortName", LabelRef::ShortName }
});

MNX_ENUM_MAPPING(LayoutStemDirection, {
    { "down",   LayoutStemDirection::Down },
    { "up",     LayoutStemDirection::Up }
});

#endif // DOXYGEN_SHOULD_IGNORE_THIS

} // namespace mnx
