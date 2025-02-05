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

#include <memory>
#include <unordered_map>
#include <cassert>

#include <nlohmann/json.hpp>

namespace mnx {

using json = nlohmann::ordered_json;

class Object;
template <typename T> class Array;

/**
 * @brief Base class wrapper for all MNX JSON nodes.
 */
class Base
{
public:
    virtual ~Base() = default;

    /**
     * @brief Convert this element for retrieval.
     *
     * Generally, you should not call this directly, but it must be public for the child setter macros
     *
     * @return A reference to the JSON node.
     */
    json& ref() const { return m_json_ref.get(); }

    /**
     * @brief Access the JSON node for modification.
     * @return A reference to the JSON node.
     */
    json& ref() { return m_json_ref.get(); }

protected:
    /**
     * @brief Wrap a Base instance around a specific JSON reference.
     * @param json_ref Reference to a JSON node.
     */
    Base(json& json_ref) : m_json_ref(json_ref) {}

    /**
     * @brief Construct a Base reference as a child inside a parent node.
     * @param json_ref Rvalue reference to a new JSON object or array.
     * @param parent_ref Reference to the parent JSON node.
     * @param key The key under which the new node is stored.
     */
    Base(json&& json_ref, json& parent_ref, const std::string_view& key)
        : m_json_ref(parent_ref[key] = std::move(json_ref)) // Move json_ref and bind m_json_ref to the new value
    {}

    /**
     * @brief Retrieves and validates a required child node.
     * @tparam T The expected MNX type (`Object` or `Array<T>`).
     * @param key The key of the child node.
     * @return An instance of the requested type.
     * @throws std::runtime_error if the key is missing or the type is incorrect.
     */
    template <typename T>
    T get_child(const std::string_view& key) const
    {
        if (!checkKeyIsValid<T>(key)) {
            throw std::runtime_error("Missing required child node: " + std::string(key));
        }
        return T(ref()[key]);
    }

    /**
     * @brief Retrieves an optional child node.
     * @tparam T The expected MNX type (`Object` or `Array<T>`).
     * @param key The key of the child node.
     * @return An `std::optional<T>`, or `std::nullopt` if the node does not exist or is invalid.
     * @throws std::runtime_error if the the type is incorrect.
     */
    template <typename T>
    std::optional<T> get_optional_child(const std::string_view& key) const
    {
        if (!checkKeyIsValid<T>(key)) {
            return std::nullopt;
        }
        return T(ref()[key]);
    }

private:
    template <typename T>
    bool checkKeyIsValid(const std::string_view& key) const
    {
        if (!ref().contains(key)) {
            return false;
        }

        if constexpr (std::is_base_of_v<Object, T>) {
            if (!ref()[key].is_object()) {
                throw std::runtime_error("Expected an object for: " + std::string(key));
            }
        } else if constexpr (std::is_base_of_v<Array<typename T::value_type>, T>) {
            if (!ref()[key].is_array()) {
                throw std::runtime_error("Expected an array for: " + std::string(key));
            }            
        }

        return true;
    }
    
    std::reference_wrapper<json> m_json_ref;
};

class Document;
/**
 * @brief Represents an MNX object, encapsulating property access.
 */
class Object : public Base
{
public:
    /// @brief Wraps an Object class around an existing JSON object element
    /// @param json_ref Reference to the element
    Object(json& json_ref) : Base(json_ref)
    {
        if (!json_ref.is_object()) {
            throw std::invalid_argument("mnx::Object must wrap a JSON object.");
        }
    }

    /// @brief Creates a new Object class as a child of a JSON element
    /// @param parent_ref The parent JSON element
    /// @param key The JSON key to use for embedding the new array.
    Object(json& parent_ref, const std::string_view& key)
        : Base(json::object(), parent_ref, key) {}

private:
    // Special constructor that defers validation for Document
    struct DeferValidation {};
    Object(json& jsonRef, DeferValidation) : Base(jsonRef) {}
    friend class Document;
};

/**
 * @brief Represents an MNX array, encapsulating property access.
 */
template <typename T>
class Array : public Base
{
    static_assert(std::is_same_v<T, int> || std::is_same_v<T, double> ||
                  std::is_same_v<T, bool> || std::is_same_v<T, std::string> ||
                  std::is_base_of_v<Base, T>, "Invalid MNX array element type.");

public:
    /// @brief The type for elements in this Array.
    using value_type = T;

    /// @brief Wraps an Array class around an existing JSON array element
    /// @param json_ref Reference to the element
    Array(json& json_ref) : Base(json_ref)
    {
        if (!json_ref.is_array()) {
            throw std::invalid_argument("mnx::Array must wrap a JSON object.");
        }
    }

    /// @brief Creates a new Array class as a child of a JSON element
    /// @param parent_ref The parent JSON element
    /// @param key The JSON key to use for embedding the new array.
    Array(json& parent_ref, const std::string_view& key)
        : Base(json::array(), parent_ref, key) {}

    /** @brief Get the size of the array. */
    size_t size() const { return ref().size(); }

    /** @brief Check if the array is empty. */
    bool empty() const { return ref().empty(); }

    /** @brief Clear all elements. */
    void clear() { ref().clear(); }

    /// @brief const operator[]
    T operator[](size_t index) const
    {
        checkIndex(index);
        if constexpr (std::is_base_of_v<Base, T>) {
            return T(ref()[index]);
        } else {
            return ref().at(index).template get<T>();
        }
    }

    /// @brief non-const operator[]
    auto operator[](size_t index)
    {
        checkIndex(index);
        if constexpr (std::is_base_of_v<Base, T>) {
            return T(ref()[index]);
        } else {
            return ref().at(index).template get_ref<T&>();
        }
    }

    /** @brief Append a new value to the array. */
    void push_back(const T& value)
    {
        ref().push_back(value);
    }

    /** @brief Remove an element at a given index. */
    void erase(size_t index)
    {
        checkIndex(index);
        ref().erase(ref().begin() + index);
    }

    /// @brief Returns an iterator to the beginning of the array.
    auto begin() { return ref().begin(); }

    /// @brief Returns an iterator to the end of the array.
    auto end() { return ref().end(); }

    /// @brief Returns a const iterator to the beginning of the array.
    auto begin() const { return ref().cbegin(); }

    /// @brief Returns a const iterator to the end of the array.
    auto end() const { return ref().cend(); }

private:
    void checkIndex(size_t index) const
    {
        assert(index < ref().size());
        if (index >= ref().size()) {
            throw std::out_of_range("Index out of range");
        }
    }
};

#define MNX_REQUIRED_PROPERTY(PARENT_TYPE, TYPE, NAME) \
    struct __##NAME##_Wrapper__ { \
        PARENT_TYPE& self; \
        TYPE operator()() const { \
            if (!self.ref().contains(#NAME)) { \
                throw std::runtime_error("Missing required property: " #NAME); \
            } \
            return self.ref()[#NAME].get<TYPE>(); \
        } \
        void set(const TYPE& value) { self.ref()[#NAME] = value; } \
    }; \
    friend struct __##NAME##_Wrapper__; \
    __##NAME##_Wrapper__ NAME{ *this }

#define MNX_OPTIONAL_PROPERTY(PARENT_TYPE, TYPE, NAME) \
    struct __##NAME##_Wrapper__ { \
        PARENT_TYPE& self; \
        std::optional<TYPE> operator()() const { \
            return self.ref().contains(#NAME) ? std::optional<TYPE>(self.ref()[#NAME].get<TYPE>()) : std::nullopt; \
        } \
        void set(const TYPE& value) { self.ref()[#NAME] = value; } \
        void clear() { self.ref().erase(#NAME); } \
    }; \
    friend struct __##NAME##_Wrapper__; \
    __##NAME##_Wrapper__ NAME{ *this }

#define MNX_REQUIRED_CHILD(PARENT_TYPE, TYPE, NAME) \
    struct __##NAME##_Wrapper__ { \
        PARENT_TYPE& self; \
        TYPE operator()() const { return self.get_child<TYPE>(#NAME); } \
        void set(const TYPE& value) { self.ref()[#NAME] = value.ref(); } \
    }; \
    friend struct __##NAME##_Wrapper__; \
    __##NAME##_Wrapper__ NAME{ *this }

#define MNX_OPTIONAL_CHILD(PARENT_TYPE, TYPE, NAME) \
    struct __##NAME##_Wrapper__ { \
        PARENT_TYPE& self; \
        std::optional<TYPE> operator()() const { return self.get_optional_child<TYPE>(#NAME); } \
        void set(const TYPE& value) { self.ref()[#NAME] = value.ref(); } \
        void clear() { self.ref().erase(#NAME); } \
    }; \
    friend struct __##NAME##_Wrapper__; \
    __##NAME##_Wrapper__ NAME{ *this }

} // namespace mnx
