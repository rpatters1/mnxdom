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
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate and return no error";

    const auto mnx = doc.mnx();
    EXPECT_EQ(mnx.version(), 1);
    ASSERT_TRUE(mnx.support().has_value());
    auto support = *mnx.support();
    EXPECT_TRUE(support.useAccidentalDisplay());
    support.set_useAccidentalDisplay(false);
    EXPECT_FALSE(support.useAccidentalDisplay());

    auto measures = doc.global().measures();
    EXPECT_EQ(measures.size(), 0u);
    measures.append();
    ASSERT_EQ(doc.global().measures().size(), 1u);
    measures[0].set_index(3);  // use non const operator[]
    const auto measuresConst = doc.global().measures();
    const auto measure = measuresConst[0]; // use const operator[]
    EXPECT_EQ(measure.index(), 3);
    EXPECT_EQ(doc.global().measures()[0].index(), 3);

    EXPECT_EQ(doc.parts().size(), 0u);
}

TEST(Document, MinimalFromScratch)
{
    Document doc;
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate and return no error";

    auto mnx = doc.mnx();
    EXPECT_EQ(mnx.version(), MNX_VERSION);
    mnx.set_version(MNX_VERSION + 1);
    EXPECT_EQ(doc.mnx().version(), MNX_VERSION + 1);

    auto support = mnx.create_support();
    support.set_useAccidentalDisplay(true);

    EXPECT_TRUE(support.useAccidentalDisplay()) << "mnx has a support instance";
    doc.mnx().clear_support();
    EXPECT_THROW(static_cast<void>(support.useAccidentalDisplay()), json::out_of_range)
            << "document no longer has a support instance, so the support instance is stale";
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate without a support instance";

    auto parts = doc.parts();
    EXPECT_EQ(parts.size(), 0u);
    auto part = parts.append();
    EXPECT_EQ(doc.parts().size(), 1u);
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate after adding a part";
    auto measures = part.create_measures();
    measures.append();
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate after adding a measure to a part";

    auto layouts = doc.create_layouts();
    layouts.append();
    auto layout = layouts[0];
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate after adding a layout, even with no layout id";
    layout.set_id("layout0");
    auto content = layout.content();
    auto staff = content.append<layout::Staff>();
    staff.set_symbol(LayoutSymbol::Bracket);
    EXPECT_EQ(staff.symbol(), LayoutSymbol::Bracket);
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate after adding a layout";
    //std::cout << doc.dump(4) << std::endl;
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
    EXPECT_FALSE(validation::schemaValidate(doc)) << "schema should not validate";

    auto mnx = doc.mnx();
    EXPECT_THROW(static_cast<void>(mnx.version()), std::runtime_error);
    mnx.set_version(MNX_VERSION);
    EXPECT_EQ(doc.mnx().version(), MNX_VERSION);
    EXPECT_FALSE(validation::schemaValidate(doc)) << "after adding version, schema should still not validate";

    auto global = doc.global();
    EXPECT_THROW(static_cast<void>(global.measures()), std::runtime_error);
    global.create_measures();
    EXPECT_EQ(doc.global().measures().size(), 0u);
    EXPECT_TRUE(validation::schemaValidate(doc)) << "after adding global, schema should validate";

    EXPECT_EQ(doc.parts().size(), 0u);
    auto part = doc.parts().append();
    EXPECT_TRUE(validation::schemaValidate(doc)) << "after adding part, schema should validate";
    EXPECT_EQ(part.staves(), 1);
    ASSERT_EQ(doc.parts().size(), 1u);
    doc.parts()[0].set_staves(3);
    doc.parts()[0].set__c("This is a comment.");
    EXPECT_EQ(part.staves(), 3) << "detached instance should reflect changed value from doc";
    EXPECT_TRUE(validation::schemaValidate(doc)) << "after adding comment, schema should validate";
}
