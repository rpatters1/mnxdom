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

using namespace mnx;

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
    Document doc(jsonString);

    const auto mnx = doc.mnx();
    EXPECT_FALSE(doc.validate().has_value()) << "schema should validate and return no error";

    EXPECT_EQ(mnx.version(), 1);
    ASSERT_TRUE(mnx.support().has_value());
    auto support = *mnx.support();
    ASSERT_TRUE(support.useAccidentalDisplay().has_value());
    EXPECT_TRUE(*support.useAccidentalDisplay());
    support.set_useAccidentalDisplay(false);
    EXPECT_FALSE(*support.useAccidentalDisplay());

    auto measures = doc.global().measures();
    EXPECT_EQ(measures.size(), 0);
    auto measure = measures.append();
    EXPECT_EQ(doc.global().measures().size(), 1);

    EXPECT_EQ(doc.parts().size(), 0);
}

TEST(Document, MinimalFromScratch)
{
    Document doc;
    EXPECT_FALSE(doc.validate().has_value()) << "schema should validate and return no error";

    auto mnx = doc.mnx();
    EXPECT_EQ(mnx.version(), MNX_VERSION);
    mnx.set_version(MNX_VERSION + 1);
    EXPECT_EQ(doc.mnx().version(), MNX_VERSION + 1);

    auto support = mnx.create_support();
    support.set_useAccidentalDisplay(false);

    EXPECT_TRUE(support.useAccidentalDisplay()) << "mnx has a support instance";
    doc.mnx().clear_support();
    EXPECT_THROW(support.useAccidentalDisplay(), json::out_of_range)
            << "document no longer has a support instance, so the support instance is stale";
    EXPECT_FALSE(doc.validate().has_value()) << "schema should validate without a support instance";
}

TEST(Document, MissingRequiredFields)
{
    std::istringstream jsonString(R"(
        {
            "mnx": {
            },
            "global": {
            },
            "parts": []
        }
    )");
    Document doc(jsonString);
    EXPECT_TRUE(doc.validate().has_value()) << "schema should not validate";

    auto mnx = doc.mnx();
    EXPECT_THROW(mnx.version(), std::runtime_error);
    mnx.set_version(MNX_VERSION);
    EXPECT_EQ(doc.mnx().version(), MNX_VERSION);
    EXPECT_TRUE(doc.validate().has_value()) << "after adding version, schema should still not validate";

    auto global = doc.global();
    EXPECT_THROW(global.measures(), std::runtime_error);
    global.create_measures();
    EXPECT_EQ(doc.global().measures().size(), 0);
    EXPECT_FALSE(doc.validate().has_value()) << "after adding global, schema should validate";

    EXPECT_EQ(doc.parts().size(), 0);
}
