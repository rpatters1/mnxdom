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

void setupTestDataPaths()
{
    auto workingDir = std::filesystem::current_path();
    ASSERT_TRUE(std::filesystem::is_directory(workingDir));
    ASSERT_GE(std::distance(workingDir.begin(), workingDir.end()), 2);
    ASSERT_EQ((--workingDir.end())->u8string(), "data");
    ASSERT_EQ((--workingDir.parent_path().end())->u8string(), "tests");
    ASSERT_TRUE(std::filesystem::exists(getInputPath()));
    ASSERT_NO_THROW({
        auto outputDir = getOutputPath();
        if (std::filesystem::exists(outputDir)) {
            std::filesystem::remove_all(outputDir);
        }
        std::filesystem::create_directories(outputDir);
    });
    ASSERT_TRUE(std::filesystem::exists(getOutputPath()));
}

void copyInputToOutput(const std::string& fileName, std::filesystem::path& outputPath)
{
    auto inputPath = getInputPath() / fileName;
    ASSERT_TRUE(std::filesystem::exists(inputPath));
    outputPath = getOutputPath() / fileName;
    ASSERT_NO_THROW({
        std::filesystem::create_directories(outputPath.parent_path());
        std::filesystem::copy(inputPath, outputPath, std::filesystem::copy_options::overwrite_existing);
    });
    ASSERT_TRUE(std::filesystem::exists(outputPath));
}

void compareFiles(const std::filesystem::path& path1, const std::filesystem::path& path2)
{
    ASSERT_TRUE(std::filesystem::is_regular_file(path1)) << "unable to find " << path1.u8string();
    ASSERT_TRUE(std::filesystem::is_regular_file(path2)) << "unable to find " << path2.u8string();
    std::ifstream file1(path1);
    ASSERT_TRUE(file1);
    std::ifstream file2(path2);
    ASSERT_TRUE(file2);
    char c1, c2;
    while (file1.get(c1)) {
        ASSERT_TRUE(file2.get(c2));
        ASSERT_EQ(c1, c2) << "comparing " << path1.u8string() << " and " << path2.u8string();
    }
    EXPECT_FALSE(file2.get(c2));
}

void assertStringsInFile(const std::vector<std::string>& targets, const std::filesystem::path& filePath, const std::filesystem::path& extension)
{
    std::filesystem::path actualFilePath;
    if (std::filesystem::is_regular_file(filePath)) {
        actualFilePath = filePath;
    } else if (std::filesystem::is_directory(filePath)) {
        auto it = std::filesystem::directory_iterator(filePath);
        auto matchingFile = std::find_if(begin(it), end(it), [&extension](const std::filesystem::directory_entry& entry) {
            return entry.is_regular_file() && entry.path().extension() == extension;
        });
        ASSERT_NE(matchingFile, std::filesystem::end(it)) << "No file with extension " << extension.u8string() << " found in directory: " << filePath.u8string();
        actualFilePath = matchingFile->path();
    } else {
        FAIL() << "Path is neither a regular file nor a directory: " << filePath.u8string();
    }
    std::ifstream file(actualFilePath, std::ios::binary);
    ASSERT_TRUE(file.is_open()) << "failed to open file: " << actualFilePath.u8string();
    std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    for (const auto& target : targets) {
        if (target.size() > 1 && target[0] == '!') {
            EXPECT_EQ(fileContents.find(target.substr(1)), std::string::npos)
                << "String \"" << target.substr(1) << "\" was not expected but was found in file: " << actualFilePath.u8string();
        } else {
            EXPECT_NE(fileContents.find(target), std::string::npos)
                << "String \"" << target << "\" not found in file: " << actualFilePath.u8string();
        }
    }
}

bool validateSchema(const mnx::Document& doc, const std::filesystem::path& filePath)
{
    auto schemaResult = mnx::validation::schemaValidate(doc);
    EXPECT_TRUE(schemaResult) << "schema validation failed for " << filePath.filename();
    if (!schemaResult) {
        for (const auto& err : schemaResult.errors) {
            std::cout << "SCHEMA ERROR " << filePath.filename() << ": " << std::endl;
            std::cout << err.to_string(4) << std::endl;
        }
        return false;
    }
    return true;
}

bool validateSemantics(const mnx::Document& doc, const std::filesystem::path& filePath)
{
    auto semanticResult = mnx::validation::semanticValidate(doc);
    EXPECT_TRUE(semanticResult) << "semantic validation failed for " << filePath.filename();
    if (!semanticResult) {
        for (const auto& err : semanticResult.errors) {
            std::cout << "SEMANTIC ERROR " << filePath.filename() << ": " << std::endl;
            std::cout << err.to_string(4) << std::endl;
        }
        return false;
    }
    return true;
}

bool fullValidate(const mnx::Document& doc, const std::filesystem::path& filePath)
{
    if (validateSchema(doc, filePath)) {
        return validateSemantics(doc, filePath);
    }
    return false;
}

void expectSemanticErrors(const mnx::Document& doc, const std::filesystem::path& filePath, const std::vector<std::string>& errors)
{
    bool schemaResult = validateSchema(doc, filePath);
    EXPECT_TRUE(schemaResult) << "schema validation failed for " << filePath;
    if (!schemaResult) return;

    auto semanticResult = mnx::validation::semanticValidate(doc);
    EXPECT_FALSE(semanticResult) << "semantic validation succeeded for " << filePath.filename() << " but expected failure";

    std::vector<std::string> actualMessages;
    for (const auto& err : semanticResult.errors)
        actualMessages.push_back(err.message);

    bool showMessages = false;
    for (const auto& expected : errors) {
        bool found = std::any_of(actualMessages.begin(), actualMessages.end(),
                                 [&](const std::string& msg) {
                                     return msg.find(expected) != std::string::npos;
                                 });
        EXPECT_TRUE(found) << "Expected error string not found: \"" << expected << "\" in file " << filePath.filename();
        if (!found) {
            showMessages = true;
        }
    }

    if (showMessages) {
        for (const auto& err : semanticResult.errors) {
            std::cout << "SEMANTIC ERROR " << filePath.filename() << ": " << std::endl;
            std::cout << err.to_string(4) << std::endl;
        }
    }
}

// Optional setup/teardown for test suite
class TestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        // Code to run before all tests
    }

    void TearDown() override {
        // Code to run after all tests
    }
};

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new TestEnvironment);
    return RUN_ALL_TESTS();
}
