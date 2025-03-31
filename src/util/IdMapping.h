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
class mapping_error : public std::exception
{
private:
    std::string message;

    template <typename KeyType>
    static std::string format_key_string(const KeyType& key) {
        if constexpr (std::is_same_v<KeyType, std::string>) {
            return "\"" + key + "\"";
        } else {
            std::ostringstream oss;
            oss << key;
            return oss.str();
        }
    }

protected:
    /// @brief Constructor
    template <typename KeyType>
    mapping_error(const KeyType& key, const std::string& suffix)
        : message("ID " + format_key_string(key) + " " + suffix) {}

public:
    /// @brief Returns the error message
    const char* what() const noexcept override
    {
        return message.c_str();
    }
};

/// @brief Exception when mapping is not found
class mapping_not_found : public mapping_error
{
public:
    /// @brief Constructor
    template <typename KeyType>
    explicit mapping_not_found(const KeyType& key)
        : mapping_error(key, "not found in ID mapping")
    {
    }
};

/// @brief Exception when an attempt is made to add a duplicate key.
class mapping_duplicate : public mapping_error
{
public:
    /// @brief Constructor
    template <typename KeyType>
    explicit mapping_duplicate(const KeyType& key, json_pointer currentValue)
        : mapping_error(key, "already exists at " + currentValue.to_string())
    {
    }
};

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
// Helper to produce static_assert failure for unsupported types
template <typename T>
inline constexpr bool always_false = false;
#endif // DOXYGEN_SHOULD_IGNORE_THIS

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
     */
    explicit IdMapping(std::shared_ptr<json> documentRoot)
        : m_root(documentRoot) {}

    /**
     * @brief Looks up an object by string ID.
     * @tparam T The expected type (e.g., mnx::Part, mnx::Layout, mnx::sequence::Note).
    /// @tparam IdType The type of @p id
     * @param id The ID to search for.
     * @return An instance of T if found.
     * @throws mapping_not_found if the ID is not found or mismatched.
     */
    template <typename T, typename IdType>
    T find(const IdType& id) const
    {
        const auto& map = getMap<T>();
        auto it = map.find(id);
        if (it == map.end()) {
            throw mapping_not_found(id);
        }
        return T(m_root, it->second);
    };

    /// @brief Adds a key to the mapping
    /// @tparam T The type to add
    /// @tparam IdType The type of @p id
    /// @param id The ID to add.
    /// @param value The value to index.
    /// @throws mapping_duplicate if the ID is a duplicate.
    template <typename T, typename IdType>
    void add(const IdType& id, const T& value)
    {
        auto& map = getMap<T>();
        auto it = map.find(id);
        if (it != map.end()) {
            throw mapping_duplicate(id, value.pointer());
        }
        map.emplace(id, value.pointer());
    }

    /// @brief Returns the @ref part::Measure instance associated with the input @ref global::Measure.
    /// @param globalMeasure The global measure for which to find the associated part measure.
    /// @param partId ID of the part to search.
    /// @return The associated part measure.
    mnx::part::Measure getPartMeasure(const mnx::global::Measure& globalMeasure, const std::string& partId) const
    {
        const size_t measureIndex = globalMeasure.calcArrayIndex();
        const auto part = find<mnx::Part>(partId);
        const auto measures = part.measures();
        if (!measures || measureIndex >= measures.value().size()) {
            throw std::invalid_argument("Part \'" + partId + "\" lacks a corresponding measure for the input global measure: " + globalMeasure.pointer().to_string());
        }
        return measures.value()[measureIndex];
    }

private:
    std::shared_ptr<json> m_root;

    using Map = std::unordered_map<std::string, json_pointer>;
    Map m_eventMap;
    Map m_layoutMap;
    Map m_noteMap;
    Map m_partMap;

    std::unordered_map<int, json_pointer> m_globalMeasures;

    template <typename T, typename Self>
    static auto& getMapImpl(Self& self) {
        if constexpr (std::is_same_v<T, mnx::Part>) {
            return self.m_partMap;
        } else if constexpr (std::is_same_v<T, mnx::Layout>) {
            return self.m_layoutMap;
        } else if constexpr (std::is_same_v<T, mnx::sequence::Note>) {
            return self.m_noteMap;
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
};

} // namespace mnx::util
