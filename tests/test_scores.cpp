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

TEST(Scores, FromScratch)
{
    Document doc;
    EXPECT_FALSE(doc.validate().has_value()) << "schema should validate and return no error";

    static constexpr int numBars = 5;
    for (int x = 0; x < numBars; x++) {
        doc.global().measures().append();
    }
    EXPECT_EQ(doc.global().measures().size(), numBars);

    auto scores = doc.create_scores();

    auto score = scores.append("Full Score");
    auto pages = score.create_pages();
    auto page = pages.append();
    auto system = page.systems().append(1);
    EXPECT_EQ(system.measure(), 1);
    EXPECT_EQ(page.systems().append(4).measure(), 4);
    auto layoutChanges = system.create_layoutChanges();
    auto layoutChange = layoutChanges.append("layout1", 2, 3, 8);
    EXPECT_EQ(layoutChange.layout(), "layout1");
    EXPECT_EQ(layoutChange.location().bar(), 2);

    auto frac = layoutChange.location().position().fraction();
    EXPECT_EQ(frac.numerator(), 3);
    EXPECT_EQ(frac.denominator(), 8);

    frac.set_numerator(5);
    EXPECT_EQ(frac.numerator(), 5);

    EXPECT_FALSE(doc.validate().has_value()) << "schema should validate after adding a score";
    std::cout << doc.dump(4) << std::endl;
}