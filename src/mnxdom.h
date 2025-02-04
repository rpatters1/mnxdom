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

/** @mainpage
 * Welcome to the <b>MNX Document Model</b>.
 *
 * Use the <b>navigation buttons</b> at the top of this page to browse through
 * the framework documentation.
 *
 * The MNX Document Model is a <b>C++ class framework</b>
 * around the MNX music interchange format, which uses JSON files.
 * It requires the C++17 standard.
 * 
 * Features include:
 * - Dependency-free class definitions.
 * - Factory classes separate from object model classes.
 *
 * The following macro definitions are available to modify behavior of the library.
 * - `MNX_THROW_ON_VALIDATION_ERROR`: Throws `mnx::dom::validation_error` if a class fails its integrity check.
 * Otherwise it it logs the message, which by default sends it to `std::cerr`.
 *
 * The recommended way to define these macros is from your make file or build project. They are primarily intended
 * for debugging.
 *
 * @author Robert Patterson
 */

#pragma once

#include "BaseTypes.h"