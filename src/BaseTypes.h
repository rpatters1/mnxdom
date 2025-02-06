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
#include <iostream>

#include "nlohmann/json.hpp"

#define MNX_REQUIRED_PROPERTY(TYPE, NAME) \
    TYPE NAME() const { \
        if (!ref().contains(#NAME)) { \
            throw std::runtime_error("Missing required property: " #NAME); \
        } \
        return ref()[#NAME].get<TYPE>(); \
    } \
    void set_##NAME(const TYPE& value) { ref()[#NAME] = value; } \
    static_assert(true, "") // require semicolon after macro

#define MNX_OPTIONAL_PROPERTY(TYPE, NAME) \
    std::optional<TYPE> NAME() const { \
        return ref().contains(#NAME) ? std::optional<TYPE>(ref()[#NAME].get<TYPE>()) : std::nullopt; \
    } \
    void set_##NAME(const TYPE& value) { ref()[#NAME] = value; } \
    void clear_##NAME() { ref().erase(#NAME); } \
    static_assert(true, "") // require semicolon after macro

#define MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(TYPE, NAME, DEFAULT) \
    TYPE NAME() const { \
        return ref().contains(#NAME) ? ref()[#NAME].get<TYPE>() : DEFAULT; \
    } \
    void set_##NAME(const TYPE& value) { ref()[#NAME] = value; } \
    void clear_##NAME() { ref().erase(#NAME); } \
    static_assert(true, "")

#define MNX_REQUIRED_CHILD(TYPE, NAME) \
    TYPE NAME() const { return getChild<TYPE>(#NAME); } \
    void set_##NAME(const TYPE& value) { setChild(#NAME, value); } \
    TYPE create_##NAME() { return setChild(#NAME, TYPE(*this, #NAME)); } \
    static_assert(true, "") // require semicolon after macro

#define MNX_OPTIONAL_CHILD(TYPE, NAME) \
    std::optional<TYPE> NAME() const { return getOptionalChild<TYPE>(#NAME); } \
    void set_##NAME(const TYPE& value) { setChild(#NAME, value); } \
    TYPE create_##NAME() { return setChild(#NAME, TYPE(*this, #NAME)); } \
    void clear_##NAME() { ref().erase(#NAME); } \
    static_assert(true, "") // require semicolon after macro

namespace mnx {

inline constexpr int MNX_VERSION = 1;

using json = nlohmann::ordered_json;        ///< JSON class for MNX
using json_pointer = json::json_pointer;    ///< JSON pointer class for MNX

class Object;
template <typename T> class Array;

/**
 * @brief Base class wrapper for all MNX JSON nodes.
 */
class Base
{
public:
    virtual ~Base() = default;

    /// @brief Dumps the branch to a string. Useful in debugging.
    /// @param indents Number of indents or -1 for no indents 
    std::string dump(int indents = -1)
    {
        return ref().dump(indents);
    }

protected:
    /**
     * @brief Convert this element for retrieval.
     *
     * @return A reference to the JSON node.
     */
    json& ref() const { return resolve_pointer(); }

    /**
     * @brief Access the JSON node for modification.
     * @return A reference to the JSON node.
     */
    json& ref() { return resolve_pointer(); }

    /// @brief Returns the json_pointer for this node.
    json_pointer pointer() const { return m_pointer; }

    /// @brief Returns the parent object for this node
    /// @tparam T The type to create. Must correctly match whether it is an array or object.
    /// @throws std::invalid_argument if the type of T does not match the type of the underlying pointer.
    template <typename T>
    T parent() const
    {
        static_assert(std::is_base_of_v<Base, T>, "Template type mush be derived from Base.");
        return T(m_root, m_pointer.parent_pointer());
    }

    /**
     * @brief Wrap a Base instance around a specific JSON reference using a json_pointer.
     * @param root Reference to the root JSON object.
     * @param pointer JSON pointer to the specific node.
     */
    Base(const std::reference_wrapper<json>& root, json_pointer pointer)
        : m_root(root), m_pointer(std::move(pointer)) {}

    /**
     * @brief Construct a Base reference as a child inside a parent node.
     * @param jsonRef Rvalue reference to a new JSON object or array.
     * @param parent Reference to the parent instance.
     * @param key The key under which the new node is stored.
     */
    Base(json&& jsonRef, Base& parent, const std::string_view& key)
        : m_root(parent.m_root), m_pointer(parent.m_pointer / std::string(key))
    {
        m_root.get()[m_pointer] = std::move(jsonRef);
    }

    /**
     * @brief Retrieves and validates a required child node.
     * @tparam T The expected MNX type (`Object` or `Array<T>`).
     * @param key The key of the child node.
     * @return An instance of the requested type.
     * @throws std::runtime_error if the key is missing or the type is incorrect.
     */
    template <typename T>
    T getChild(const std::string_view& key) const
    {
        static_assert(std::is_base_of_v<Base, T>, "template type must be derived from Base");

        json_pointer childPointer = m_pointer / std::string(key);
        if (!checkKeyIsValid<T>(childPointer)) {
            throw std::runtime_error("Missing required child node: " + std::string(key));
        }

        return T(m_root, childPointer);
    }

    /**
     * @brief Sets a child node.
     * @tparam T The expected MNX type (`Object` or `Array<T>`).
     * @param key The key of the child node.
     * @param value The value to set.
     * @return The newly created child.
     */
    template <typename T>
    T setChild(const std::string_view& key, const T& value)
    {
        static_assert(std::is_base_of_v<Base, T>, "template type must be derived from Base");

        json_pointer childPointer = m_pointer / std::string(key);
        m_root.get()[childPointer] = value.ref();
        return T(m_root, childPointer);
    }

    /**
     * @brief Retrieves an optional child node.
     * @tparam T The expected MNX type (`Object` or `Array<T>`).
     * @param key The key of the child node.
     * @return An `std::optional<T>`, or `std::nullopt` if the node does not exist or is invalid.
     * @throws std::runtime_error if the type is incorrect.
     */
    template <typename T>
    std::optional<T> getOptionalChild(const std::string_view& key) const
    {
        static_assert(std::is_base_of_v<Base, T>, "template type must be derived from Base");

        json_pointer childPointer = m_pointer / std::string(key);
        if (!checkKeyIsValid<T>(childPointer)) {
            return std::nullopt;
        }

        return T(m_root, childPointer);
    }

private:
    /**
     * @brief Checks whether a key is valid for the given type.
     * @tparam T The expected MNX type.
     * @param pointer JSON pointer to the key.
     * @return True if the key is valid, false otherwise.
     * @throws std::runtime_error if the type is incorrect.
     */
    template <typename T>
    bool checkKeyIsValid(const json_pointer& pointer) const
    {
        if (!m_root.get().contains(pointer)) {
            return false;
        }

        const json& node = m_root.get().at(pointer);

        if constexpr (std::is_base_of_v<Object, T>) {
            if (!node.is_object()) {
                throw std::runtime_error("Expected an object for: " + pointer.to_string());
            }
        } else if constexpr (std::is_base_of_v<Array<typename T::value_type>, T>) {
            if (!node.is_array()) {
                throw std::runtime_error("Expected an array for: " + pointer.to_string());
            }
        }

        return true;
    }

    /**
     * @brief Resolves the JSON node using the stored pointer.
     * @return Reference to the JSON node.
     * @throws json::out_of_range if the node does not exist.
     */
    json& resolve_pointer() const
    {
        return m_root.get().at(m_pointer);  // Throws if invalid
    }

    std::reference_wrapper<json> m_root;    ///< Reference to the root JSON object.
    json_pointer m_pointer;                 ///< JSON pointer to the specific node.
};

class Document;
/**
 * @brief Represents an MNX object, encapsulating property access.
 */
class Object : public Base
{
public:
    /// @brief Wraps an Object class around an existing JSON object element
    /// @param root Reference to the document root
    /// @param pointer The json_pointer value for the element
    Object(json& root, json_pointer pointer) : Base(root, pointer)
    {
        if (!ref().is_object()) {
            throw std::invalid_argument("mnx::Object must wrap a JSON object.");
        }
    }

    /// @brief Creates a new Object class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    Object(Base& parent, const std::string_view& key)
        : Base(json::object(), parent, key) {}

private:
    // Special constructor that defers validation for Document
    Object(json& root) : Base(root, json_pointer{}) {}
    friend class Document;
};

/**
 * @brief Represents an MNX object that is included as an array element.
 */
class ArrayElementObject : public Object
{
public:
    using Object::Object;

    size_t calcArrayIndex() const
    {
        return std::stoul(pointer().back());
    }
};

/**
 * @brief Represents an MNX array, encapsulating property access.
 */
template <typename T>
class Array : public Base
{
    static_assert(std::is_same_v<T, int> || std::is_same_v<T, double> ||
                  std::is_same_v<T, bool> || std::is_same_v<T, std::string> ||
                  std::is_base_of_v<ArrayElementObject, T>, "Invalid MNX array element type.");

public:
    /// @brief The type for elements in this Array.
    using value_type = T;

    /// @brief Wraps an Array class around an existing JSON array element
    /// @param root Reference to the document root
    /// @param pointer The json_pointer value for the element
    Array(json& root, json_pointer pointer) : Base(root, pointer)
    {
        if (!ref().is_array()) {
            throw std::invalid_argument("mnx::Array must wrap a JSON array.");
        }
    }

    /// @brief Creates a new Array class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    Array(Base& parent, const std::string_view& key)
        : Base(json::array(), parent, key) {}

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
            return getChild<T>(std::to_string(index));
        } else {
            return ref().at(index).template get<T>();
        }
    }

    /// @brief non-const operator[]
    auto operator[](size_t index)
    {
        checkIndex(index);
        if constexpr (std::is_base_of_v<Base, T>) {
            return getChild<T>(std::to_string(index));
        } else {
            return ref().at(index).template get_ref<T&>();
        }
    }

    /** @brief Append a new value to the array. (Available only for primitive types) */
    template <typename U = T>
    std::enable_if_t<!std::is_base_of_v<Base, U>, void>
    push_back(const U& value)
    {
        ref().push_back(value);
    }

    /**
     * @brief Create a new element at the end of the array. (Available only for Base types)
     * @return The newly created element.
    */
    template <typename U = T, std::enable_if_t<std::is_base_of_v<Base, U>, int> = 0>
    U append()
    {
        if constexpr (std::is_base_of_v<Object, U>) {
            ref().push_back(json::object());
        } else {
            ref().push_back(json::array());
        }
        return U(*this, std::to_string(ref().size() - 1));
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

protected:
    void checkIndex(size_t index) const
    {
        assert(index < ref().size());
        if (index >= ref().size()) {
            throw std::out_of_range("Index out of range");
        }
    }
};

class ContentObject : ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    MNX_REQUIRED_PROPERTY(std::string, type);   ///< determines our type in the JSON
};

class ContentArray : public Array<ContentObject> {
public:
    using BaseArray = Array<ContentObject>;
    using BaseArray::BaseArray;  // Inherit constructors

    /// @brief Retrieve an element from the array as a specific type
    template <typename T, std::enable_if_t<std::is_base_of_v<ContentObject, T>, int> = 0>
    T get(size_t index) const
    {
        return getTypedObject<T>(index);
    }

private:
    /// @brief Constructs an object of type `T` if its type matches the JSON type
    /// @throws std::invalid_argument if there is a type mismatch
    template <typename T, std::enable_if_t<std::is_base_of_v<ContentObject, T>, int> = 0>
    T getTypedObject(size_t index)
    {
        this->checkIndex(index);
        auto element = (*this)[index];
        if (element.type() != T::ContentTypeValue) {
            throw std::invalid_argument("Type mismatch: expected " + std::string(T::ContentTypeValue) +
                                        ", got " + element.type());
        }
        return T(*this, std::to_string(index));
    }
};

} // namespace mnx
