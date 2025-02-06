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
#pragma once

#include <filesystem>
#include <fstream>

#include "BaseTypes.h"

namespace mnx {

/**
 * @class MnxMetaData
 * @brief Represents metadata for an MNX document.
 */
class MnxMetaData : public Object
{
public:
    using Object::Object;

    /**
     * @class Support
     * @brief Represents optional support metadata within an MNX document.
     */
    class Support : public Object
    {
    public:
        using Object::Object;

        /**
         * @brief Optional property indicating whether accidental display is used.
         */
        MNX_OPTIONAL_PROPERTY(bool, useAccidentalDisplay);
    };

    /**
     * @brief Required property indicating the version of the MNX document.
     */
    MNX_REQUIRED_PROPERTY(int, version);

    /**
     * @brief Optional child containing support metadata.
     */
    MNX_OPTIONAL_CHILD(Support, support);
};

/**
 * @class Document
 * @brief Represents an MNX document and provides methods for loading and saving.
 */
class Document : public Object
{    
public:
    /**
     * @brief Constructs an empty MNX document.
     */
    Document() : Object(m_json_root, Object::DeferValidation{}) {}

    /**
     * @brief Required child containing metadata for the MNX document.
     */
    MNX_REQUIRED_CHILD(MnxMetaData, mnx);

    /**
     * @brief Creates a Document from an input stream.
     * @param inputStream The input stream containing the MNX JSON data.
     * @return A Document instance populated with the parsed data.
     */
    static Document create(std::istream& inputStream)
    {
        Document result;
        inputStream >> result.m_json_root;
        return result;
    }

    /**
     * @brief Creates a Document from a JSON file.
     * @param inputPath The path to the JSON file.
     * @return A Document instance populated with the parsed data.
     * @throws std::runtime_error if the file cannot be opened.
     */
    static Document create(const std::filesystem::path& inputPath)
    {
        std::ifstream jsonFile;
        jsonFile.exceptions(std::ios::failbit | std::ios::badbit);
        jsonFile.open(inputPath);
        if (!jsonFile.is_open()) {
            throw std::runtime_error("Unable to open JSON file: " + inputPath.u8string());
        }
        return create(jsonFile);
    }

    /**
     * @brief Saves the MNX document to a file.
     * @param outputPath The file path to save the document.
     * @param indentSpaces Optional number of spaces for indentation; if not provided, no indentation is applied.
     * @throws std::ios_base::failure if file operations fail.
     */
    void save(const std::filesystem::path& outputPath, std::optional<int> indentSpaces) const
    {
        std::ofstream file;
        file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        file.open(outputPath, std::ios::out | std::ios::binary);
        file << m_json_root.dump(indentSpaces.value_or(-1));
        file.close();
    }

    /**
     * @brief Validates the MNX document against a schema.
     * @param jsonSchema A string containing the schema json.
     * @returns std::nullopt if no error or an error message if there was one.
     */
    std::optional<std::string> validate(const std::optional<std::string>& jsonSchema = std::nullopt) const;

private:
    /**
     * @brief The root JSON object storing the MNX document data.
     */
    json m_json_root = json::object();
};

} // namespace mnx
