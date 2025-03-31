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
#include <filesystem>
#include <iterator>

#include "gtest/gtest.h"
#include "mnxdom.h"
#include "test_utils.h"

TEST(Layouts, DuplicateId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "duplicate_layouts.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "ID \"S0-ScrVw\" already exists at index 0");
}

TEST(Layouts, NonexistentPartId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "layout_with_bad_part.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "ID \"P-does-not-exist\" not found in ID mapping");
}

TEST(Layouts, NonexistentStaffNumber)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "layout_invalid_staffnum.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath,  "ID \"P2\" not found in ID mapping");
}
