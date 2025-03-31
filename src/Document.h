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
#include "Global.h"
#include "Layout.h"
#include "Part.h"
#include "Score.h"

namespace mnx {

namespace util {
class IdMapping;
}

/**
 * @class MnxMetaData
 * @brief Represents metadata for an MNX document.
 */
class MnxMetaData : public Object
{
public:
    using Object::Object;

    /// @brief Creates a new MnxMetaData class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    MnxMetaData(Base& parent, const std::string_view& key)
        : Object(parent, key)
    {
        // required children
        set_version(MNX_VERSION);
    }

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
private:
    /// @brief Not really shared, but std::unique_ptr creates unacceptable dependencies in the headers
    std::shared_ptr<util::IdMapping> m_idMapping;

public:
    /**
     * @brief Constructs an empty MNX document. The resulting instance contains all
     * required fields and should validate against the schema.
     */
    Document() : Object(std::make_shared<json>(json::object()), json_pointer{})
    {
        // create required children
        create_mnx();
        create_global();
        create_parts();
    }

    /// @brief Wrap a document around a root element
    /// @param root 
    Document(const std::shared_ptr<json>& root) : Object(root, json_pointer{}) {}

    /// @brief Copy constructor that zaps the id mapping, if any
    Document(const Document& src) : Object(src), m_idMapping(nullptr) {}

    using Base::root;

    /**
     * @brief Constructs a Document from an input stream.
     * @param inputStream The input stream containing the MNX JSON data.
     */
    Document(std::istream& inputStream) : Object(std::make_shared<json>(json::object()), json_pointer{})
    {
        inputStream >> *root();
    }

    MNX_REQUIRED_CHILD(Global, global);         ///< Global data for the MNX document.
    MNX_REQUIRED_CHILD(MnxMetaData, mnx);       ///< Metadata for the MNX document.
    MNX_OPTIONAL_CHILD(Array<Layout>, layouts); ///< List of layouts for the MNX document.
    MNX_REQUIRED_CHILD(Array<Part>, parts);     ///< List of parts for the MNX document.
    MNX_OPTIONAL_CHILD(Array<Score>, scores);   ///< List of scores for the MNX document.

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
        return Document(jsonFile);
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
        if (!file.is_open()) {
            throw std::runtime_error("Unable to write to JSON file: " + outputPath.u8string());
        }
        file << root()->dump(indentSpaces.value_or(-1));
        file.close();
    }

    /// @brief Allows retrieval of any node within the document typed as the given class.
    /// @tparam T The class to wrap around the pointer. No error checking is performed.
    /// @param jsonPointer The pointer to the object
    /// @return An instance of class T.
    template <typename T>
    T get(const json_pointer& jsonPointer)
    {
        static_assert(std::is_base_of_v<Base, T>, "template class must be derived from mnx::Base.");
        return T(root(), jsonPointer);
    }

    /// @brief String version of get
    /// @param jsonPointerString The pointer to the object in a string
    template <typename T>
    T get(const std::string& jsonPointerString)
    { return get<T>(json_pointer(jsonPointerString)); }

    /// @brief Builds or rebuilds the ID mapping for the document, replacing any existing mapping.
    /// @param errorHandler An optional error handler. If provided, the function does not throw on duplicate keys added.
    /// @throws util::mapping_error on duplicate keys if no @p errorHandler is provided.
    void buildIdMapping(const std::optional<ErrorHandler>& errorHandler = std::nullopt);

    /// @brief Gets a reference to the ID mapping instance for the document.
    const util::IdMapping& getIdMapping() const
    {
        MNX_ASSERT_IF(!m_idMapping) {
            throw std::logic_error("Call buildIdMapping before calling getIdMapping.");
        }
        return *m_idMapping;
    }

    /// @brief Returns whether am ID mapping currently exists
    bool hasIdMapping() const { return static_cast<bool>(m_idMapping); }
};

static_assert(std::is_move_constructible<mnx::Document>::value, "Document must be move constructible");

} // namespace mnx
