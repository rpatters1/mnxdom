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

TEST(Document, Minimal)
{
    std::istringstream jsonString(R"(
        {
            "mnx": {
                "version": 1,
                "support": {
                    "useAccidentalDisplay" : true
                }
            },
            "global": {
              "measures": []
            },
            "parts": []
        }
    )");
    auto doc = mnx::Document::create(jsonString);

    const auto mnx = doc.mnx();
    EXPECT_EQ(mnx.version(), 1);
    ASSERT_TRUE(mnx.support().has_value());
    auto support = *mnx.support();
    ASSERT_TRUE(support.ref().is_object());
    ASSERT_TRUE(support.useAccidentalDisplay().has_value());
    EXPECT_TRUE(*support.useAccidentalDisplay());
    support.set_useAccidentalDisplay(false);
    EXPECT_FALSE(*support.useAccidentalDisplay());
}