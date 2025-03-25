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

TEST(Scores, InvalidScoreLayoutId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_bad_layout.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "ID \"does-not-exist\" not found in key index list");
}

TEST(Scores, InvalidScorePageLayoutId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_page_bad_layout.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, { "ID \"does-not-exist\" not found", "ID 1 not found" });
}

TEST(Scores, InvalidScoreSystemLayoutId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_system_bad_layout.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "ID \"does-not-exist\" not found");
}

TEST(Scores, InvalidScoreLayoutChangeLayoutId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_layoutchange_bad_layout.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, { "ID 1 not found ", "ID \"does-not-exist\" not found" });
}

TEST(Scores, InvalidMMRestStartMeasure)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_mmrest_bad_start.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "ID 3 not found");
}

TEST(Scores, InvalidMMRestSpan)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_mmrest_bad_span.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "Multimeasure rest at measure 1 in score \"Score\" spans non-existent measures");
}

TEST(Scores, InvalidSystemStartMeasure)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_system_bad_measure.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "ID 1 not found");
}

TEST(Scores, InvalidSystemMeasureSequence)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_system_bad_start.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "Score \"Score\" contains system that starts before previous system");
}

TEST(Scores, InvalidSystemMeasureSequence2)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "score_system_bad_start2.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "The first system in score \"Score\" starts after the first measure");
}

TEST(Scores, EdgeCaseMMRest)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "mmrest_edgecase.json";
    auto doc = mnx::Document::create(inputPath);
    fullValidate(doc, inputPath);
}
