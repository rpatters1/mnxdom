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

#include "BaseTypes.h"

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

#define MNX_EMPTY_FIELDS(M)

// Update this list when ContentArray gains or loses ContentObject types.
// The second argument is 1 if the type has ctor fields, 0 if it has none.
// This header must be included after Sequence/Layout/Part headers define the types and field macros.
#define MNX_CONTENT_ARRAY_TYPES(M) \
    M(mnx::sequence::Event, 0, MNX_EMPTY_FIELDS) \
    M(mnx::sequence::Space, 1, MNX_SEQUENCE_SPACE_FIELDS) \
    M(mnx::sequence::Grace, 0, MNX_EMPTY_FIELDS) \
    M(mnx::sequence::MultiNoteTremolo, 1, MNX_SEQUENCE_MULTI_NOTE_TREMOLO_FIELDS) \
    M(mnx::sequence::Tuplet, 1, MNX_SEQUENCE_TUPLET_FIELDS) \
    M(mnx::layout::Staff, 0, MNX_EMPTY_FIELDS) \
    M(mnx::layout::Group, 0, MNX_EMPTY_FIELDS)

namespace mnx {

#define MNX_FIELDS_AS_TYPES_WITH_COMMA_IF_0(FIELDS)
#define MNX_FIELDS_AS_TYPES_WITH_COMMA_IF_1(FIELDS) , MNX_FIELDS_AS_DECAYED_TYPES(FIELDS)
#define MNX_FIELDS_AS_TYPES_WITH_COMMA_IF(HAS_ARGS, FIELDS) \
    MNX_PP_CAT(MNX_FIELDS_AS_TYPES_WITH_COMMA_IF_, HAS_ARGS)(FIELDS)

#define MNX_CONTENT_ARRAY_APPEND_SPECIALIZATION(TYPE, HAS_ARGS, FIELDS) \
    template <> \
    inline TYPE ContentArray::append<TYPE MNX_FIELDS_AS_TYPES_WITH_COMMA_IF(HAS_ARGS, FIELDS)>(MNX_FIELDS_AS_DECAYED_PARAMS(FIELDS)) \
    { \
        return appendImpl<TYPE>(MNX_FIELDS_AS_NAMES(FIELDS)); \
    }

MNX_CONTENT_ARRAY_TYPES(MNX_CONTENT_ARRAY_APPEND_SPECIALIZATION)

#undef MNX_CONTENT_ARRAY_APPEND_SPECIALIZATION
#undef MNX_FIELDS_AS_TYPES_WITH_COMMA_IF
#undef MNX_FIELDS_AS_TYPES_WITH_COMMA_IF_1
#undef MNX_FIELDS_AS_TYPES_WITH_COMMA_IF_0

} // namespace mnx

#undef MNX_CONTENT_ARRAY_TYPES
#undef MNX_EMPTY_FIELDS

#endif // DOXYGEN_SHOULD_IGNORE_THIS
