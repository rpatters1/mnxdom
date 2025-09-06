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

TEST(Global, DuplicateMeasures)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "duplicate_measures.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "ID 1 already exists for type \"measure-global\" at /global/measures/0");
}

TEST(Global, InvalidLyricLineIds)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "nonexistent_lyric_line_id.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, { "ID \"line1\" not found in ID mapping", "ID \"line2\" not found in ID mapping", });
}

TEST(Parts, MeasureIndices)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "measures_mismatch.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "contains a different number of measures (4) than are defined globally (3)");

    auto measures = doc.global().measures();
    int firstIndex = 1;
    for (const auto measure : measures) {
        EXPECT_EQ(measure.calcMeasureIndex(), firstIndex++);
    }
}
