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

#include <string>
#include <vector>
#include <unordered_set>

#include "BaseTypes.h"

namespace mnx {

class Document;

/**
 * @namespace mnx::validation
 * @brief provides schema- and semantic-level validation functions for MNX documents
 */
namespace validation {

/// @struct ValidationResult
/// @brief Encapsulates a schema validation result
struct ValidationResult
{
    /// @struct Error
    /// @brief All the information about a specific error
    struct Error
    {
        /// @brief constructor
        Error(const json_pointer& ptr, const json& inst, const std::string& msg)
            : instance(inst), pointer(ptr), message(msg) {}
            
        json instance;          ///< instance containing the error
        json_pointer pointer;   ///< the location of the instance in the top level
        std::string message;    ///< a message describing the error

        /// @brief Converts the error to a string. (Matches the schema validator's default message.)
        /// @param indent The number of indents in the output message. Omit for none.
        std::string to_string(int indent = -1) const
        {
            return "At " + pointer.to_string() + " of " + instance.dump(indent) + " - " + message;
        }
    };
    std::vector<Error> errors;  ///< errors encountered

    /// @brief Allows a simple if check to see if the schema validated
    explicit operator bool() const { return errors.empty(); }
};

/// @struct SemanticValidationResult
/// @brief Returns the results of semantic validation
struct SemanticValidationResult : public ValidationResult
{
    using ValidationResult::ValidationResult;

    std::unordered_map<int, size_t> measureList;        ///< key: measId; value: index of measure in `global.measures()` array
    size_t measureCount{};                              ///< count can be different than `measureList.size()` if there is a duplicate key error
    std::unordered_map<std::string, size_t> partList;   ///< list of part ID values and their indices in the `parts()` array.
    std::unordered_map<std::string, size_t> layoutList; ///< list of layout ID values and their indices in the `layouts()` array.
    std::unordered_map<std::string, size_t> lyricLines; ///< list of lyric line ID values and their indices in the `global.lyrics().lineOrder()` array.
    std::unordered_map<std::string, json_pointer> eventList; ///< list of events in the document.
    std::unordered_map<std::string, json_pointer> noteList; ///< list of notes in the document.
    std::unordered_set<std::string> notesWithTies; ///< list of notes with ties (forward) in the document
};

/// @brief Validates a document against a JSON schema
/// @param document The mnx::Document to validate
/// @param jsonSchema The JSON schema to validate against, or std::nullopt for the embedded schema
/// @return validation result
ValidationResult schemaValidate(const Document& document, const std::optional<std::string>& jsonSchema = std::nullopt);

/// @brief Validates the semantics of the input MNX document
/// @param document The document to validate
/// @return validation result
SemanticValidationResult semanticValidate(const Document& document);

} // namespace validation
} // namespace mnx