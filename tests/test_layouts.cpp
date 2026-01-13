/*
 * Copyright (C) 2024, Robert Patterson
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
#include "gtest/gtest.h"

#include "mnxdom.h"

#include "test_utils.h"

using namespace mnx;

TEST(Layouts, FullScoreDetect)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "chamberorch.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(fullValidate(doc, inputPath));

    auto fullScoreLayout = doc.findFullScoreLayout();
    ASSERT_TRUE(fullScoreLayout);
    EXPECT_EQ(fullScoreLayout->id_or(""), "S0-ScrVw");
}

TEST(Layouts, BuildLayoutSpans)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "layoutspans.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(validateSchema(doc, inputPath)) << "schema validatation failed"; // empty group means semantic fails: tested elsewhere

    doc.buildEntityMap();
    auto layout = doc.getEntityMap().tryGet<mnx::Layout>("S0-ScrVw");
    ASSERT_TRUE(layout);

    auto spans = util::buildLayoutSpans(layout.value());
    // Expected (after sorting):
    // Group A: [0,3], depth 0
    // Staff S0: [0,0], depth 2
    // Group B: [1,2], depth 1
    // Staff S1: [1,1], depth 3
    // Staff S2: [2,2], depth 3
    // Staff S3: [3,3], depth 2
    ASSERT_EQ(spans->size(), 6u);

    const auto& s = *spans;

    EXPECT_EQ(s[0].kind, mnx::util::LayoutSpan::Kind::Group);
    EXPECT_EQ(s[0].startIndex, 0u);
    EXPECT_EQ(s[0].endIndex, 3u);
    EXPECT_EQ(s[0].depth, 0u);
    ASSERT_TRUE(s[0].label.has_value());
    EXPECT_EQ(*s[0].label, "A");

    EXPECT_EQ(s[1].kind, mnx::util::LayoutSpan::Kind::Staff);
    EXPECT_EQ(s[1].startIndex, 0u);
    EXPECT_EQ(s[1].endIndex, 0u);
    EXPECT_EQ(s[1].depth, 2u);
    ASSERT_TRUE(s[1].label.has_value());
    EXPECT_EQ(*s[1].label, "S0");

    EXPECT_EQ(s[2].kind, mnx::util::LayoutSpan::Kind::Group);
    EXPECT_EQ(s[2].startIndex, 1u);
    EXPECT_EQ(s[2].endIndex, 2u);
    EXPECT_EQ(s[2].depth, 1u);
    ASSERT_TRUE(s[2].label.has_value());
    EXPECT_EQ(*s[2].label, "B");

    EXPECT_EQ(s[3].kind, mnx::util::LayoutSpan::Kind::Staff);
    EXPECT_EQ(s[3].startIndex, 1u);
    EXPECT_EQ(s[3].endIndex, 1u);
    EXPECT_EQ(s[3].depth, 3u);
    ASSERT_TRUE(s[3].label.has_value());
    EXPECT_EQ(*s[3].label, "S1");

    EXPECT_EQ(s[4].kind, mnx::util::LayoutSpan::Kind::Staff);
    EXPECT_EQ(s[4].startIndex, 2u);
    EXPECT_EQ(s[4].endIndex, 2u);
    EXPECT_EQ(s[4].depth, 3u);
    ASSERT_TRUE(s[4].label.has_value());
    EXPECT_EQ(*s[4].label, "S2");

    EXPECT_EQ(s[5].kind, mnx::util::LayoutSpan::Kind::Staff);
    EXPECT_EQ(s[5].startIndex, 3u);
    EXPECT_EQ(s[5].endIndex, 3u);
    EXPECT_EQ(s[5].depth, 2u);
    ASSERT_TRUE(s[5].label.has_value());
    EXPECT_EQ(*s[5].label, "S3");
}
