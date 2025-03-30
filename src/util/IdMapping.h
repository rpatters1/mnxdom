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

#include "BaseTypes.h"
#include "Global.h"
#include "Layout.h"
#include "Part.h"
#include "Score.h"

namespace mnx::util {

class mapping_not_found : public std::exception
{
private:
    std::string message;

public:    
    template <typename KeyType>
    mapping_not_found(const KeyType& key) : exception()
    {
        std::string keyString = [key]() {
            if constexpr (std::is_same_v<KeyType, std::string>) {
                return "\"" + key + "\"";
            } else {
                return std::to_string(key);
            }
        }();
        message = "ID " + keyString + " not found in ID mapping.");
    }

    const char* what() const noexcept override
    {
        return message.c_str();
    }
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
     */
    explicit IdMapping(std::shared_ptr<json> documentRoot);

    /**
     * @brief Looks up an object by string ID.
     * @tparam T The expected type (e.g., mnx::Part, mnx::Layout, mnx::sequence::Note).
     * @param id The string ID to search for.
     * @return An instance of T if found.
     * @throws std::out_of_range if the ID is not found or mismatched.
     */
    template <typename T, typename IdType>
    T find(const IdType& id) const;

private:
    std::shared_ptr<nlohmann::json> m_root;

    using Map = std::unordered_map<std::string, json_pointer>;
    Map m_eventMap;
    Map m_layoutMap;
    Map m_noteMap;
    Map m_partMap;

    std::unordered_map<int, json_pointer> m_globalMeasures;
    std::unordered_map<std::string, std::unordered_map<int, json_pointer>> m_partMeasures;
};

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
// Helper to produce static_assert failure for unsupported types
template <typename T>
inline constexpr bool always_false = false;
#endif // DOXYGEN_SHOULD_IGNORE_THIS

template <typename T, typename IdType>
T IdMapping::find(const IdType& id) const
{
    const auto& map = [&]() -> const auto& {
        if constexpr (std::is_same_v<T, mnx::Part>) {
            return m_partMap;
        } else if constexpr (std::is_same_v<T, mnx::Layout>) {
            return m_layoutMap;
        } else if constexpr (std::is_same_v<T, mnx::sequence::Note>) {
            return m_noteMap;
        } else if constexpr (std::is_same_v<T, mnx::sequence::Event>) {
            return m_eventMap;
        } else if constexpr (std::is_same_v<T, mnx::global::Measure>) {
            return m_globalMeasures;
        } else {
            static_assert(always_false<T>, "Unsupported type for IdMapping::find");
        }
    }();

    auto it = map.find(id);
    if (it == map.end()) {
        throw mapping_not_found(id);
    }
    return T(m_root, it->second);
};

} // namespace mnx::util
