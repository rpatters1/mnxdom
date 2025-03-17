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

TEST(Global, LyricLineMetadata)
{
    std::istringstream jsonString(R"(
        {
            "mnx": {
                "version": 1
            },
            "global": {
                "lyrics": {
                    "lineMetadata": {
                        "3": {
                            "label": "日本語",
                            "lang": "ja"
                        },
                        "2": {
                            "label": "Deutsch",
                            "lang": "de"
                        },
                        "1": {
                            "label": "English",
                            "lang": "en"
                        },
                        "4": {
                            "label": "Español",
                            "lang": "es"
                        }
                    },
                    "lineOrder": [
                        "1",
                        "2",
                        "3",
                        "4"
                    ]
                },
              "measures": []
            },
            "parts": []
        }
    )");
    Document doc(jsonString);
    EXPECT_FALSE(doc.validate().has_value()) << "schema should validate and return no error";

    auto lyrics = doc.global().lyrics();
    ASSERT_TRUE(lyrics.has_value()) << "lyrics should have a value";
    auto lineMetaData = lyrics.value().lineMetadata();
    ASSERT_TRUE(lineMetaData.has_value()) << "lineMetaData should have a value";

    EXPECT_EQ(lineMetaData.value()["2"].lang(), "de");
    EXPECT_EQ(lineMetaData.value()["3"].label(), "日本語");

    size_t index = 0;
    for (const auto& it : lineMetaData.value()) {
        if (index == 2) {
            EXPECT_EQ(it.first, "1");
            EXPECT_EQ(it.second.lang().value(), "en");
            EXPECT_EQ(it.second.label().value(), "English");
        } else if (index == 3) {
            EXPECT_EQ(it.first, "4");
            EXPECT_EQ(it.second.lang().value(), "es");
            EXPECT_EQ(it.second.label().value(), "Español");
        }
        index++;
        ASSERT_TRUE(index <= 4) << "iteration loop is not stopping";        
        //std::cout << "\"" << it.first << "\": " << *it.second.lang() << " " << *it.second.label() << std::endl;
    }

    auto newLine = lineMetaData.value().append("15");
    newLine.set_label("Italiano");
    newLine.set_lang("it");
    EXPECT_EQ(lineMetaData.value().size(), 5u);
    EXPECT_EQ(lineMetaData.value()["15"].label(), "Italiano");
    EXPECT_EQ(lineMetaData.value()["15"].lang(), "it");

    for (auto it : lineMetaData.value()) {
        if (it.first == "3") {
            it.second.set_label("Nederlands");
            it.second.set_lang("nl");
        }
    }
    EXPECT_EQ(lineMetaData.value()["3"].label(), "Nederlands");
    EXPECT_EQ(lineMetaData.value()["3"].lang(), "nl");

    {
        const auto it = lineMetaData.value().find("222");
        EXPECT_EQ(it, lineMetaData.value().end()) << "find invalid key";
    }
    {
        const auto it = lineMetaData.value().find("2");
        EXPECT_NE(it, lineMetaData.value().end()) << "find valid key";
        EXPECT_EQ(it->second.label(), "Deutsch");
        EXPECT_EQ(it->second.lang(), "de");
    }
    {
        auto it = lineMetaData.value().find("222");
        EXPECT_EQ(it, lineMetaData.value().end()) << "find invalid key";
    }
    {
        auto it = lineMetaData.value().find("15");
        EXPECT_NE(it, lineMetaData.value().end()) << "find valid key";
        it->second.set_label("Français");
        it->second.set_lang("fr");
    }
    EXPECT_EQ(lineMetaData.value()["15"].label(), "Français");
    EXPECT_EQ(lineMetaData.value()["15"].lang(), "fr");

    EXPECT_FALSE(doc.validate().has_value()) << "schema should validate and return no error";
    //std::cout << doc.dump(4) << std::endl;
}