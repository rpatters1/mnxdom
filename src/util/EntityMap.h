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

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>

#include "BaseTypes.h"
#include "Global.h"
#include "Layout.h"
#include "Part.h"
#include "Score.h"
#include "Sequence.h"

namespace mnx::util {

namespace detail {
#ifndef DOXYGEN_SHOULD_IGNORE_THIS

template <typename T>
struct JsonSchemaTypeNames
{
    static constexpr std::array<std::string_view, 1> value{ T::JsonSchemaTypeName };
};

template <>
struct JsonSchemaTypeNames<sequence::NoteBase>
{
    static constexpr std::array<std::string_view, 2> value{
        sequence::Note::JsonSchemaTypeName,
        sequence::KitNote::JsonSchemaTypeName
    };
};

template <typename T>
bool matchesTypeName(std::string_view typeName)
{
    for (const auto name : JsonSchemaTypeNames<T>::value) {
        if (name == typeName) {
            return true;
        }
    }
    return false;
}

template <typename T>
std::string typeNamesExpectationString()
{
    std::ostringstream oss;
    const auto& names = JsonSchemaTypeNames<T>::value;
    const auto count = names.size();
    if (count > 1) {
        oss << "expected one of ";
    } else {
        oss << "expected ";
    }
    for (size_t i = 0; i < count; ++i) {
        if (i > 0) {
            oss << ", ";
        }
        oss << "\"" << names[i] << "\"";
    }
    return oss.str();
}

#endif // DOXYGEN_SHOULD_IGNORE_THIS
} // namespace detail

/// @brief base class for mapping error exceptions
class mapping_error : public std::runtime_error
{
    using runtime_error::runtime_error;
};

/**
 * @class EntityMap
 * @brief Provides type-safe ID-based lookup for elements in an MNX document.
 *
 * Constructed from an mnx::Document, the EntityMap scans the document to index
 * all identifiable elements by ID or number. Supports lookup by type.
 */
class EntityMap
{
private:
    /// @brief Adds a key to the mapping. If there is no error handler, it throws @ref mapping_error if there is a duplicate key.
    /// @tparam T The type to add
    /// @tparam IdType The type of @p id
    /// @param id The ID to add.
    /// @param value The value to index.
    /// @throws mapping_error if the ID is a duplicate and there is no error handler.
    template <typename T, typename IdType>
    void add(const IdType& id, const T& value)
    {
        auto result = getMap<T>().emplace(id, MappedLocation{ value.pointer(), T::JsonSchemaTypeName });
        if (!result.second) {
            mapping_error err("ID " + formatKeyString(id) + " already exists for type \"" + std::string(result.first->second.typeName)
                + "\" at " + result.first->second.location.to_string());
            if (m_errorHandler) {
                m_errorHandler.value()(err.what(), value);
            } else {
                throw err;
            }
        }
    }
    
    /// @brief Map an event's id to its beam
    /// @param eventId The id of the event to map.
    /// @param beam The beam that includes the event.
    void addEventToBeam(const std::string& eventId, const part::Beam& beam)
    {
        auto result = m_eventsInBeams.emplace(eventId, BeamMappingEntry{ MappedLocation{ beam.pointer(), part::Beam::JsonSchemaTypeName }, 0 });
        if (!result.second) {
            mapping_error err("ID " + formatKeyString(eventId) + " already exists in beam "
                + result.first->second.location.location.to_string());
            if (m_errorHandler) {
                m_errorHandler.value()(err.what(), beam);
            } else {
                throw err;
            }
        }
    }

    /// @brief Record (or lower) the beam depth that starts at an event.
    void setEventBeamStartLevel(const std::string& eventId, int level)
    {
        auto it = m_eventsInBeams.find(eventId);
        if (it == m_eventsInBeams.end()) {
            throw std::logic_error("Attempted to assign a beam start level to unmapped event " + eventId);
        }
        if (it->second.startLevel == 0) {
            it->second.startLevel = level;
        } else {
            it->second.startLevel = std::min(it->second.startLevel, level);
        }
    }

    /// @brief Cache the ottava shift for a specific event pointer.
    void setEventOttavaShift(const std::string& eventPointer, int shift)
    {
        m_eventOttavaShift[eventPointer] = shift;
    }

    friend class mnx::Document;

public:
    /**
     * @brief Constructs the index for a given document.
     * @param documentRoot Shared pointer to the document's JSON root.
     * @param errorHandler A optional callback function for handling errors.
     */
    explicit EntityMap(std::weak_ptr<json> documentRoot, const std::optional<ErrorHandler>& errorHandler = std::nullopt)
        : m_root(documentRoot), m_errorHandler(errorHandler) {}
        
    /**
     * @brief Attempts to look up an object by string ID.
     * @tparam T The expected type (e.g., mnx::Part, mnx::Layout, mnx::sequence::Note).
     * @tparam IdType The type of @p id.
     * @param id The ID to search for.
     * @param errorLocation The location in the document for error reporting purposes.
     * @return An instance of T if found; std::nullopt if the ID is not present.
     *
     * @throws mapping_error if the ID is found but has a different type than @p T
     *         (release builds only; debug builds assert instead).
     *
     * @note A type mismatch indicates an internal logic error in the ID mapping.
     *       This function only models the *absence* of an ID, not type ambiguity.
     */
    template <typename T, typename IdType>
    std::optional<T> tryGet(
        const IdType& id,
        const std::optional<Base>& errorLocation = std::nullopt) const
    {
        const auto& map = getMap<T>();
        auto it = map.find(id);
        if (it == map.end()) {
            return std::nullopt;
        }
        MNX_ASSERT_IF(!detail::matchesTypeName<T>(it->second.typeName)) {
            mapping_error err(
                "ID " + formatKeyString(id) + " has type \"" + std::string(it->second.typeName) +
                "\", but " + detail::typeNamesExpectationString<T>() + "."
            );
            if (m_errorHandler) {
                m_errorHandler.value()(err.what(), errorLocation.value_or(Document(root())));
            }
            throw err;
        }
        return T(root(), it->second.location);
    }

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
        if (auto v = tryGet<T>(id, errorLocation)) {
            return *std::move(v);
        }
        mapping_error err("ID " + formatKeyString(id) + " not found in ID mapping");
        if (m_errorHandler) {
            m_errorHandler.value()(err.what(), errorLocation.value_or(Document(root())));
        }
        throw err;
    }

    /// @brief Returns the array index of an object identified by ID.
    ///
    /// Looks up an object of type @p T by its ID and returns the index of that
    /// object within its owning array. This is equivalent to calling #get<T>()
    /// followed by #ArrayElementObject::calcArrayIndex().
    ///
    /// @tparam T The expected object type. Must derive from ArrayElementObject.
    /// @tparam IdType The type of the ID used for lookup.
    /// @param id The ID of the object to locate.
    /// @param errorLocation Optional document location used for error reporting.
    /// @return The zero-based array index of the object.
    /// @throws mapping_error if the ID is not found or does not refer to an object of type @p T.
    template <typename T, typename IdType>
    size_t getIndexOf(const IdType& id, const std::optional<Base>& errorLocation = std::nullopt) const
    {
        static_assert(std::is_base_of_v<ArrayElementObject, T>,
                    "getIndexOf<T> requires T to derive from ArrayElementObject");

        auto v = get<T>(id, errorLocation);
        return v.calcArrayIndex();
    }


    /// @brief Returns whether the specified ID exists in the mapping
    template <typename T, typename IdType>
    bool exists(const IdType& id) const
    {
        const auto& map = getMap<T>();
        return map.find(id) != map.end();
    }

    /// @brief Get the beam for an event, if it is mapped.
    /// @param event The event to search for.
    /// @return The beam or std::nullopt if not found.
    std::optional<part::Beam> tryGetBeam(const sequence::Event& event) const
    {
        if (const auto& eventId = event.id()) {
            const auto it = m_eventsInBeams.find(eventId.value());
            if (it != m_eventsInBeams.end()) {
                MNX_ASSERT_IF(it->second.location.typeName != part::Beam::JsonSchemaTypeName) {
                    mapping_error err(
                        "The beam mapping for eventId " + formatKeyString(eventId.value())
                        + " was mapped to an object of type \"" + std::string(it->second.location.typeName) + "\"."
                    );
                    if (m_errorHandler) {
                        m_errorHandler.value()(err.what(), event);
                    }
                    throw err;
                }
                return part::Beam(root(), it->second.location.location);
            }
        }
        return std::nullopt;
    }

    /// @brief Return the secondary beam depth that starts at an event ID, if any.
    [[nodiscard]] std::optional<int> tryGetBeamStartLevel(const std::string& eventId) const
    {
        const auto it = m_eventsInBeams.find(eventId);
        if (it == m_eventsInBeams.end()) {
            return std::nullopt;
        }
        return it->second.startLevel;
    }

    /// @brief Return the beam start level for an event ID or 0 if none.
    [[nodiscard]] int getBeamStartLevel(const std::string& eventId) const
    {
        if (const auto level = tryGetBeamStartLevel(eventId)) {
            return *level;
        }
        return 0;
    }

    /// @brief Clears all mapped items.
    void clear()
    {
        m_objectMap.clear();
        m_globalMeasures.clear();
        m_eventsInBeams.clear();
        m_eventOttavaShift.clear();
        m_lyricLineOrder.clear();
    }

    /// @brief Retrieve the ottava shift for an event (if known).
    [[nodiscard]] std::optional<int> tryGetOttavaShift(const sequence::Event& event) const
    {
        const auto it = m_eventOttavaShift.find(event.pointer().to_string());
        if (it == m_eventOttavaShift.end()) {
            return std::nullopt;
        }
        return -it->second;
    }

    /// @brief Retrieve the ottava shift for an event. Returns 0 if not cached.
    [[nodiscard]] int getOttavaShift(const sequence::Event& event) const
    {
        if (auto shift = tryGetOttavaShift(event)) {
            return shift.value();
        }
        return 0;
    }

    /// @brief Retrieve the lyric line order.
    [[nodiscard]] const std::vector<std::string>& getLyricLineOrder() const
    { return m_lyricLineOrder; }

private:
    std::weak_ptr<json> m_root;
    std::optional<ErrorHandler> m_errorHandler;

    std::shared_ptr<json> root() const
    {
        std::shared_ptr<json> result = m_root.lock();
        if (!result) {
            throw std::runtime_error("ID mapping is invalid because the document was destroyed.");
        }
        return result;
    }

    using MappedLocation = struct
    {
        json_pointer location;          ///< location of instance in JSON
        std::string_view typeName;      ///< schema name of type for this instance
    };
    std::unordered_map<std::string, MappedLocation> m_objectMap;
    std::unordered_map<int, MappedLocation> m_globalMeasures;
    struct BeamMappingEntry
    {
        MappedLocation location;
        int startLevel{0};
    };
    std::unordered_map<std::string, BeamMappingEntry> m_eventsInBeams;
    std::unordered_map<std::string, int> m_eventOttavaShift;
    std::vector<std::string> m_lyricLineOrder;

    template <typename T, typename Self>
    static auto& getMapImpl(Self& self) {
        if constexpr (std::is_same_v<T, global::Measure>) {
            return self.m_globalMeasures;
        } else {
            return self.m_objectMap;
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
