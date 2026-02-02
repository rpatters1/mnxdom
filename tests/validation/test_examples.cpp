
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
#include <array>
#include <filesystem>
#include <iterator>
#include <string_view>
#include <algorithm>

#include "gtest/gtest.h"
#include "mnxdom.h"
#include "test_utils.h"

namespace {
constexpr std::array<std::string_view, 3> kSchemaOnlyExamples = {
    "orchestral-layout.json",
    "organ-layout.json",
    "system-layouts.json"
};

bool shouldOnlySchemaValidate(const std::filesystem::path& path)
{
    const auto filename = path.filename().string();
    return std::find(kSchemaOnlyExamples.begin(), kSchemaOnlyExamples.end(), filename) != kSchemaOnlyExamples.end();
}
} // namespace

TEST(Examples, All)
{
    const std::filesystem::path inputPath = MNX_W3C_EXAMPLES_PATH;
    ASSERT_TRUE(std::filesystem::exists(inputPath)) << "examples path does not exist: " << inputPath;
    int filesProcessed = 0;
    for (const auto& entry : std::filesystem::directory_iterator(inputPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            auto doc = mnx::Document::create(entry.path());
            if (shouldOnlySchemaValidate(entry.path())) {
                validateSchema(doc, entry.path());
            } else {
                fullValidate(doc, entry.path());
            }
            filesProcessed++;
        }
    }
    EXPECT_GT(filesProcessed, 0) << "no files processed!";
}
