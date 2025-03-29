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

TEST(Parts, DuplicateId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "duplicate_parts.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "ID \"P1\" already exists at index 0");
}

TEST(Parts, MeasuresMismatch)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "measures_mismatch.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "contains a different number of measures (4) than are defined globally (3)");
}

TEST(Parts, InvalidLyricsLineIds)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "lyrics_invalid_lineids.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, { "ID \"1\" not found in key value list", "ID \"2\" not found in key value list" });
}

TEST(Parts, EventErrors)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "event_errors.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "ID \"ev5n1\" already exists at",
        "ID \"ev6\" already exists at",
        "Event \"ev7\" is a rest but also has notes",
        "Event \"ev8\" has both full measure indicator and duration"
    });
}

TEST(Parts, TieErrors)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "bad_ties.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "Tie points to a note in a different part",
        "Tie points to a note with a different pitch"
    });
}

TEST(Parts, EnharmonicTies)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "edge_cases" / "enharmonic_ties.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(fullValidate(doc, inputPath)) << "full validation";
}