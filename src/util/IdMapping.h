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

#include <unordered_map>
#include <memory>
#include <string>
#include <tuple>
#include <stdexcept>
#include <type_traits>
#include <optional>
#include <sstream>

#include "BaseTypes.h"
#include "Global.h"
#include "Layout.h"
#include "Part.h"
#include "Score.h"

namespace mnx::util {

/// @brief base class for mapping error exceptions
class mapping_error : public std::runtime_error
{
    using runtime_error::runtime_error;
};

/**
 * @class IdMapping
 * @brief Provides type-safe ID-based lookup for elements in an MNX document.
 *
 * Constructed from an mnx::Document, the IdMapping scans the document to index
 * all identifiable elements by ID or number. Supports lookup by type.
 */
class IdMapping {
public:
    /**
     * @brief Constructs the index for a given document.
     * @param documentRoot Shared pointer to the document's JSON root.
     * @param errorHandler A optional callback function for handling errors.
     */
    explicit IdMapping(std::shared_ptr<json> documentRoot, const std::optional<ErrorHandler>& errorHandler = std::nullopt)
        : m_root(documentRoot), m_errorHandler(errorHandler) {}

    /**
     * @brief Looks up an object by string ID.
     * @tparam T The expected type (e.g., mnx::Part, mnx::Layout, mnx::sequence::Note).
     * @tparam IdType The type of @p id
     * @param id The ID to search for.
     * @param errorLocation The location in the document for error reporting purposes.
     * @return An instance of T if found.
     * @throws mapping_error if the ID is not found.
     */
    template <typename T, typename IdType>
    T get(const IdType& id, const std::optional<Base>& errorLocation = std::nullopt) const
    {
        const auto& map = getMap<T>();
        auto it = map.find(id);
        if (it == map.end()) {
            mapping_error err("ID " + formatKeyString(id) + " not found in ID mapping");
            if (m_errorHandler) m_errorHandler.value()(err.what(), errorLocation.value_or(Document(m_root)));
            throw err;
        }
        return T(m_root, it->second);
    }

    /// @brief Returns whether the specified ID exists in the mapping
    template <typename T, typename IdType>
    bool exists(const IdType& id) const
    {
        const auto& map = getMap<T>();
        return map.find(id) != map.end();
    }

    /// @brief Adds a key to the mapping. If there is no error handler, it throws @ref mapping_error if there is a duplicate key.
    /// @tparam T The type to add
    /// @tparam IdType The type of @p id
    /// @param id The ID to add.
    /// @param value The value to index.
    /// @throws mapping_error if the ID is a duplicate and there is no error handler.
    template <typename T, typename IdType>
    void add(const IdType& id, const T& value)
    {
        auto result = getMap<T>().emplace(id, value.pointer());
        if (!result.second) {
            mapping_error err("ID " + formatKeyString(id) + " already exists at " + result.first->second.to_string());
            if (m_errorHandler) {
                m_errorHandler.value()(err.what(), value);
            } else {
                throw err;
            }
        }
    }

    /// @brief Returns the @ref part::Measure instance associated with the input @ref global::Measure.
    /// @param globalMeasure The global measure for which to find the associated part measure.
    /// @param partId ID of the part to search.
    /// @return The associated part measure.
    mnx::part::Measure getPartMeasure(const mnx::global::Measure& globalMeasure, const std::string& partId) const
    {
        const size_t measureIndex = globalMeasure.calcArrayIndex();
        const auto part = get<mnx::Part>(partId);
        const auto measures = part.measures();
        if (!measures || measureIndex >= measures.value().size()) {
            mapping_error err("Part \'" + partId + "\" lacks a corresponding measure for the input global measure.");
            if (m_errorHandler) m_errorHandler.value()(err.what(), globalMeasure);
            throw err;
        }
        return measures.value()[measureIndex];
    }

private:
    std::shared_ptr<json> m_root;
    std::optional<ErrorHandler> m_errorHandler;

    using Map = std::unordered_map<std::string, json_pointer>;
    Map m_eventMap;
    Map m_layoutMap;
    Map m_noteMap;
    Map m_kitNoteMap;
    Map m_partMap;

    std::unordered_map<int, json_pointer> m_globalMeasures;

    // stoopid workround
    template <typename>
    static constexpr bool always_false = false;
    
    template <typename T, typename Self>
    static auto& getMapImpl(Self& self) {
        if constexpr (std::is_same_v<T, mnx::Part>) {
            return self.m_partMap;
        } else if constexpr (std::is_same_v<T, mnx::Layout>) {
            return self.m_layoutMap;
        } else if constexpr (std::is_same_v<T, mnx::sequence::Note>) {
            return self.m_noteMap;
        } else if constexpr (std::is_same_v<T, mnx::sequence::KitNote>) {
            return self.m_kitNoteMap;
        } else if constexpr (std::is_same_v<T, mnx::sequence::Event>) {
            return self.m_eventMap;
        } else if constexpr (std::is_same_v<T, mnx::global::Measure>) {
            return self.m_globalMeasures;
        } else {
            static_assert(always_false<T>, "Unsupported type for IdMapping::getMap");
        }
    }

    template <typename T>
    const auto& getMap() const
    { return getMapImpl<T>(*this); }

    template <typename T>
    auto& getMap()
    { return getMapImpl<T>(*this); }

    template <typename KeyType>
    static std::string formatKeyString(const KeyType& key) {
        if constexpr (std::is_same_v<KeyType, std::string>) {
            return "\"" + key + "\"";
        } else {
            std::ostringstream oss;
            oss << key;
            return oss.str();
        }
    }
};

} // namespace mnx::util
