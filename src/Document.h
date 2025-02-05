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

class MnxMetaData : public Object
{
public:
    using Object::Object;

    class Support : public Object
    {
    public:
        using Object::Object;

        MNX_OPTIONAL_PROPERTY(Support, bool, useAccidentalDisplay);
    };

    MNX_REQUIRED_PROPERTY(MnxMetaData, int, version);
    MNX_OPTIONAL_CHILD(MnxMetaData, Support, support);
};

class Document : public Object
{    
public:
    Document() : Object(m_json_root, Object::DeferValidation{}) {}

    MNX_REQUIRED_CHILD(Document, MnxMetaData, mnx);

    static Document create(std::istream& inputStream)
    {
        Document result;
        inputStream >> result.m_json_root;
        return result;
    }

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

    void save(const std::filesystem::path& outputPath, std::optional<int> indentSpaces) const
    {
        std::ofstream file;
        file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        file.open(outputPath, std::ios::out | std::ios::binary);
        file << m_json_root.dump(indentSpaces.value_or(-1));
        file.close();
    }

private:
    json m_json_root = json::object();
};

} // namespace mnx
