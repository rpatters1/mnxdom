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
#include <optional>
#include <type_traits>

#include "nlohmann/json.hpp"

 /**
  * @brief creates a required property with a simple type
  *
  * It creates the following class methods.
  *
  * - `NAME()` returns the value of the property.
  * - `set_NAME(value) sets the value of the property.
  *
  * @param TYPE the type of the property
  * @param NAME the name of the property (no quotes)
  */
#define MNX_REQUIRED_PROPERTY(TYPE, NAME) \
    [[nodiscard]] TYPE NAME() const { \
        if (!ref().contains(#NAME)) { \
            throw std::runtime_error("Missing required property: " #NAME); \
        } \
        return ref()[#NAME].get<TYPE>(); \
    } \
    void set_##NAME(const TYPE& value) { ref()[#NAME] = value; } \
    static_assert(true, "") // require semicolon after macro

 /**
  * @brief creates a property (with a simple type) that occupies a fixed position in an array
  *
  * It creates the following class methods.
  *
  * - `NAME()` returns the value of the property.
  * - `set_NAME(value) sets the value of the property.
  *
  * @param TYPE the type of the property
  * @param NAME the name of the property (no quotes)
  * @param INDEX the index in the array for the property
  */
 #define MNX_ARRAY_ELEMENT_PROPERTY(TYPE, NAME, INDEX) \
    static_assert(std::is_integral_v<decltype(INDEX)>, "array index must be an integer type"); \
    [[nodiscard]] TYPE NAME() const { return (*this)[INDEX]; } \
    void set_##NAME(const TYPE& value) { (*this)[INDEX] = value; } \
    static_assert(true, "") // require semicolon after macro

 /**
  * @brief creates an optional named property with a simple type. This is a property whose name is different than
  * its JSON key. This is used when the JSON property name is a C++ keyword. An example is `class` that becomes a
  * property called `styleClass`.
  *
  * It creates the following class methods.
  *
  * - `NAME()` returns a std::optional<TYPE> containing the value of the property.
  * - `NAME_or(value)` returns the property value if it exists or the input value if not.
  * - `set_NAME(value) sets the value of the property.
  * - `clear_NAME() clears the property from the JSON document.
  *
  * @param TYPE the type of the property
  * @param NAME the name of the property (no quotes)
  * @param KEY the JSON key of the property (with quotes)
  */
 #define MNX_OPTIONAL_NAMED_PROPERTY(TYPE, NAME, KEY) \
    [[nodiscard]] std::optional<TYPE> NAME() const { \
        return ref().contains(KEY) ? std::optional<TYPE>(ref()[KEY].get<TYPE>()) : std::nullopt; \
    } \
    [[nodiscard]] TYPE NAME##_or(const TYPE& defaultVal) const { \
        return ref().contains(KEY) ? ref()[KEY].get<TYPE>() : defaultVal; \
    } \
    void set_##NAME(const TYPE& value) { ref()[KEY] = value; } \
    void clear_##NAME() { ref().erase(KEY); } \
    static_assert(true, "") // require semicolon after macro

 /**
  * @brief creates an optional property with a simple type.
  *
  * It creates the following class methods.
  *
  * - `NAME()` returns a std::optional<TYPE> containing the value of the property.
  * - `NAME_or(value)` returns the property value if it exists or the input value if not.
  * - `set_NAME(value) sets the value of the property.
  * - `clear_NAME() clears the property from the JSON document.
  *
  * @param TYPE the type of the property
  * @param NAME the name of the property (no quotes)
  */
 #define MNX_OPTIONAL_PROPERTY(TYPE, NAME) MNX_OPTIONAL_NAMED_PROPERTY(TYPE, NAME, #NAME)

 /**
  * @brief creates an optional property with a default value.
  *
  * It has the following class methods.
  *
  * - `NAME()` returns the value of the property.
  * - `set_NAME(value) sets the value of the property.
  * - `clear_NAME() clears the property from the JSON document.
  * - `set_or_clear_NAME(value) sets the value of the property if the input value is not the default.
  * Otherwise it clears the property.
  *
  * @param TYPE the type of the property
  * @param NAME the name of the property (no quotes)
  * @param DEFAULT the default value of the property.
  */
#define MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(TYPE, NAME, DEFAULT) \
    [[nodiscard]] TYPE NAME() const { \
        return ref().contains(#NAME) ? ref()[#NAME].get<TYPE>() : DEFAULT; \
    } \
    void set_##NAME(const TYPE& value) { ref()[#NAME] = value; } \
    void clear_##NAME() { ref().erase(#NAME); } \
    void set_or_clear_##NAME(const TYPE& value) { \
        if (value == DEFAULT) clear_##NAME(); \
        else set_##NAME(value); \
    } \
    static_assert(true, "") // require semicolon after macro

 /**
  * @brief creates a required child object or array
  *
  * It creates the following class methods.
  *
  * - `NAME()` returns the child.
  * - `create_NAME(args...) creates the child from the input constructor arguments
  *
  * @param TYPE the type of the child object or array
  * @param NAME the name of the child object or array (no quotes)
  */
 #define MNX_REQUIRED_CHILD(TYPE, NAME) \
    [[nodiscard]] TYPE NAME() const { return getChild<TYPE>(#NAME); } \
    template<typename... Args> \
    TYPE create_##NAME(Args&&... args) { \
        return setChild(#NAME, TYPE(*this, #NAME, std::forward<Args>(args)...)); \
    } \
    static_assert(true, "") // require semicolon after macro

 /**
  * @brief creates an optional child object or array
  *
  * It creates the following class methods.
  *
  * - `NAME()` returns a std::optional<TYPE> containing the child or std::nullopt if none.
  * - `create_NAME(args...) if the child does not exist, creates the child from the input constructor arguments. Otherwise returns the child.
  * - `clear_NAME(args...) clears the child from JSON document.
  *
  * @param TYPE the type of the child object or array
  * @param NAME the name of the child object or array (no quotes)
  */
 #define MNX_OPTIONAL_CHILD(TYPE, NAME) \
    [[nodiscard]] std::optional<TYPE> NAME() const { return getOptionalChild<TYPE>(#NAME); } \
    template<typename... Args> \
    TYPE create_##NAME(Args&&... args) { \
        if (auto child = getOptionalChild<TYPE>(#NAME)) return child.value(); \
        return setChild(#NAME, TYPE(*this, #NAME, std::forward<Args>(args)...)); \
    } \
    void clear_##NAME() { ref().erase(#NAME); } \
    static_assert(true, "") // require semicolon after macro

#define MNX_ASSERT_IF(TEST) \
assert(!(TEST)); \
if (TEST)
    
/**
 * @namespace mnx
 * @brief object model for MNX format
 * 
 * See: https://w3c.github.io/mnx/docs/
 */
namespace mnx {

/// @brief The MNX version for files generated by the DOM
inline constexpr int MNX_VERSION = 1;

using json = nlohmann::ordered_json;        ///< JSON class for MNX
using json_pointer = json::json_pointer;    ///< JSON pointer class for MNX

class Object;
class Document;
template <typename T> class Array;

namespace validation {
class SemanticValidator;
}; // namespace validation

/**
 * @brief Base class wrapper for all MNX JSON nodes.
 */
class Base
{
public:
    virtual ~Base() = default;

    /// @brief Copy constructor
    Base(const Base& src) : m_root(src.m_root), m_pointer(src.m_pointer)
    {}

    /// @brief Move constructor
    Base(Base&& src) noexcept : m_root(src.m_root),    // m_root must be copied (not moved)
        m_pointer(std::move(src.m_pointer))
    {}

    /// @brief Copy assignment operator
    Base& operator=(const Base& src)
    {
        if (this != &src) {
            if (m_root != src.m_root) {
                throw std::logic_error("Assignment from a different JSON document is not allowed.");
            }
            m_pointer = src.m_pointer;
        }
        return *this;
    }

    /// @brief Move assignment operator
    Base& operator=(Base&& src)
    {
        if (this != &src) {
            if (m_root != src.m_root) {
                throw std::logic_error("Assignment from a different JSON document is not allowed.");
            }
            m_pointer = std::move(src.m_pointer);
        }
        return *this;
    }

    /// @brief Dumps the branch to a string. Useful in debugging.
    /// @param indents Number of indents or -1 for no indents 
    [[nodiscard]] std::string dump(int indents = -1) const
    {
        return ref().dump(indents);
    }

    /// @brief Returns the parent object for this node
    /// @tparam T The type to create. Must correctly match whether it is an array or object.
    /// @throws std::invalid_argument if the type of T does not match the type of the underlying pointer.
    template <typename T>
    [[nodiscard]] T parent() const
    {
        static_assert(std::is_base_of_v<Base, T>, "Template type mush be derived from Base.");
        return T(m_root, m_pointer.parent_pointer());
    }

    /// @brief Returns the enclosing array element for this instance. If T is a type that can be nested (e.g. ContentObject), the highest
    /// level instance is returned. (To get the lowest level immediate container, use #ArrayElementObject::container.)
    /// @tparam T The type to find. A limited list of types are supported, including @ref Part and @ref Sequence. Others may be added as needed.
    /// @return the enclosing element, or std::nullopt if not found.
    template <typename T>
    [[nodiscard]] std::optional<T> getEnclosingElement() const;

    /// @brief Returns the json_pointer for this node.
    [[nodiscard]] json_pointer pointer() const { return m_pointer; }

    /// @brief Returns the document root
    [[nodiscard]] Document document() const;

protected:
    /**
     * @brief Convert this node for retrieval.
     *
     * @return A reference to the JSON node.
     */
    [[nodiscard]] json& ref() const { return resolve_pointer(); }

    /**
     * @brief Access the JSON node for modification.
     * @return A reference to the JSON node.
     */
    [[nodiscard]] json& ref() { return resolve_pointer(); }

    /// @brief Returns the root.
    [[nodiscard]] const std::shared_ptr<json>& root() const { return m_root; }

    /**
     * @brief Wrap a Base instance around a specific JSON reference using a json_pointer.
     * @param root Reference to the root JSON object.
     * @param pointer JSON pointer to the specific node.
     */
    Base(const std::shared_ptr<json>& root, json_pointer pointer)
        : m_root(root), m_pointer(std::move(pointer)) {}

    /**
     * @brief Construct a Base reference as a child inside a parent node.
     * @param jsonRef Rvalue reference to a new JSON object or array.
     * @param parent Reference to the parent instance.
     * @param key The key under which the new node is stored.
     */
    Base(json&& jsonRef, Base& parent, std::string_view key)
        : m_root(parent.m_root), m_pointer(parent.m_pointer / std::string(key))
    {
        (*m_root)[m_pointer] = std::move(jsonRef);
    }

    /**
     * @brief Retrieves and validates a required child node.
     * @tparam T The expected MNX type (`Object` or `Array<T>`).
     * @param key The key of the child node.
     * @return An instance of the requested type.
     * @throws std::runtime_error if the key is missing or the type is incorrect.
     */
    template <typename T>
    [[nodiscard]] T getChild(std::string_view key) const
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
    T setChild(std::string_view key, const T& value)
    {
        static_assert(std::is_base_of_v<Base, T>, "template type must be derived from Base");

        json_pointer childPointer = m_pointer / std::string(key);
        (*m_root)[childPointer] = value.ref();
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
    [[nodiscard]] std::optional<T> getOptionalChild(std::string_view key) const
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
    [[nodiscard]] bool checkKeyIsValid(const json_pointer& pointer) const
    {
        if (!(*m_root).contains(pointer)) {
            return false;
        }

        const json& node = (*m_root).at(pointer);

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
    [[nodiscard]] json& resolve_pointer() const
    {
        return (*m_root).at(m_pointer);  // Throws if invalid
    }

    const std::shared_ptr<json> m_root;  ///< Shared pointer to the root JSON object.
    json_pointer m_pointer;          ///< JSON pointer to the specific node.

    friend class validation::SemanticValidator;
};

/// @brief Error handler type for reporting errors
using ErrorHandler = std::function<void(const std::string& message, const Base& location)>;

/**
 * @brief Represents an MNX object, encapsulating property access.
 */
class Object : public Base
{
public:
    /// @brief Wraps an Object class around an existing JSON object node
    /// @param root Reference to the document root
    /// @param pointer The json_pointer value for the node
    Object(const std::shared_ptr<json>& root, json_pointer pointer) : Base(root, pointer)
    {
        if (!ref().is_object()) {
            throw std::invalid_argument("mnx::Object must wrap a JSON object.");
        }
    }

    /// @brief Creates a new Object class as a child of a JSON node
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Object(Base& parent, std::string_view key)
        : Base(json::object(), parent, key) {}

    MNX_OPTIONAL_PROPERTY(std::string, _c);     ///< An optional comment. This serves a similar function as XML or HTML comments.
    MNX_OPTIONAL_CHILD(Object, _x);             ///< Vendor-defined dictionary.
    MNX_OPTIONAL_PROPERTY(std::string, id);     ///< Uniquely identifies the object
};

/// @brief Allows access to a fundamental type (number, boolean, string) in a JSON node
/// @tparam T The fundamental type to wrap. 
template <typename T, std::enable_if_t<!std::is_base_of_v<Base, T>, int> = 0>
class SimpleType : public Base
{
    static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, std::string>, "This template is for simple JSON classes");

public:
    using value_type = T; ///< value type of this SimpleType

    /// @brief Wraps a SimpleType class around an existing JSON object node
    /// @param root Reference to the document root
    /// @param pointer The json_pointer value for the node
    SimpleType(const std::shared_ptr<json>& root, json_pointer pointer) : Base(root, pointer)
    {
    }

    /// @brief Implicit conversion to simple type
    operator T() const
    {
        return ref().template get<T>();
    }

    /// @brief Allow assignment to underlying json reference
    /// @param src The simple type to assign.
    SimpleType& operator=(const T& src)
    {
        ref() = src;
        return *this;
    }

    /// @brief Equality comparison with value type
    bool operator==(const T& src) const
    {
        return src == ref().template get<T>();
    }
};

class ArrayElementObject;
/**
 * @brief Represents an MNX array, encapsulating property access.
 */
template <typename T>
class Array : public Base
{
    static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, std::string> ||
                  std::is_base_of_v<ArrayElementObject, T>, "Invalid MNX array element type.");

private:    
    template<typename ArrayType>
    struct iter
    {
    private:
        ArrayType* m_ptr;
        mutable size_t m_idx;

    public:
        iter(ArrayType* ptr, size_t idx) : m_ptr(ptr), m_idx(idx) {}
        T operator*() const { return (*m_ptr)[m_idx]; }
        iter& operator++() { ++m_idx; return *this; }
        bool operator!=(const iter& o) const { return m_idx != o.m_idx; }
    };

public:
    /// @brief The type for elements in this Array.
    using value_type = T;

    using iterator = iter<Array>;               ///< non-const iterator type
    using const_iterator = iter<const Array>;   ///< const iterator type

    /// @brief Wraps an Array class around an existing JSON array node
    /// @param root Reference to the document root
    /// @param pointer The json_pointer value for the node
    Array(const std::shared_ptr<json>& root, json_pointer pointer) : Base(root, pointer)
    {
        if (!ref().is_array()) {
            throw std::invalid_argument("mnx::Array must wrap a JSON array.");
        }
    }

    /// @brief Creates a new Array class as a child of a JSON node
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Array(Base& parent, std::string_view key)
        : Base(json::array(), parent, key) {}

    /** @brief Get the size of the array. */
    [[nodiscard]] size_t size() const { return ref().size(); }

    /** @brief Check if the array is empty. */
    [[nodiscard]] bool empty() const { return ref().empty(); }

    /** @brief Clear all elements. */
    void clear() { ref().clear(); }

    /** @brief Direct getter for a particular element. */
    [[nodiscard]] T at(size_t index) const
    {
        return operator[](index);
    }
    
    /// @brief const operator[]
    [[nodiscard]] auto operator[](size_t index) const
    {
        checkIndex(index);
        if constexpr (std::is_base_of_v<Base, T>) {
            return getChild<T>(std::to_string(index));
        } else {
            return getChild<SimpleType<T>>(std::to_string(index));
        }
    }

    /// @brief non-const operator[]
    [[nodiscard]] auto operator[](size_t index)
    {
        checkIndex(index);
        if constexpr (std::is_base_of_v<Base, T>) {
            return getChild<T>(std::to_string(index));
        } else {
            return getChild<SimpleType<T>>(std::to_string(index));
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
    template <typename U = T, typename... Args,
              std::enable_if_t<std::is_base_of_v<Base, U>, int> = 0>
    U append(Args&&... args)
    {
        if constexpr (std::is_base_of_v<Object, U>) {
            ref().push_back(json::object());
        } else {
            ref().push_back(json::array());
        }
        return U(*this, std::to_string(ref().size() - 1), std::forward<Args>(args)...);
    }

    /** @brief Remove an element at a given index. */
    void erase(size_t index)
    {
        checkIndex(index);
        ref().erase(ref().begin() + index);
    }

    /// @brief Returns an iterator to the beginning of the array.
    [[nodiscard]] auto begin() { return iterator(this, 0); }

    /// @brief Returns an iterator to the end of the array.
    [[nodiscard]] auto end() { return iterator(this, size()); }

    /// @brief Returns a const iterator to the beginning of the array.
    [[nodiscard]] auto begin() const { return const_iterator(this, 0); }

    /// @brief Returns a const iterator to the end of the array.
    [[nodiscard]] auto end() const { return const_iterator(this, size()); }

protected:
    /// @brief validates that an index is not out of range
    /// @throws std::out_of_range if the index is out of range
    void checkIndex(size_t index) const
    {
        assert(index < ref().size());
        if (index >= ref().size()) {
            throw std::out_of_range("Index out of range");
        }
    }
};

/**
 * @brief Represents an MNX object that is included as an array element.
 */
class ArrayElementObject : public Object
{
public:
    using Object::Object;

    /// @brief Calculates the array index of the current instance within the array.
    size_t calcArrayIndex() const
    {
        return std::stoul(pointer().back());
    }

    /// @brief Returns the container of the array this element belongs to wrapped as the specified template type.
    ///
    /// No error checking is performed beyond verifying that ContainerType matches being an array or object with the json node.
    ///
    /// @tparam ContainerType The type to wrap around the container
    template <typename ContainerType>
    ContainerType container() const
    {
        return parent<Array<ArrayElementObject>>().parent<ContainerType>();
    }
};

class ContentArray;
/// @brief Base class for objects that are elements of content arrays
class ContentObject : public ArrayElementObject
{
protected:
    static constexpr std::string_view ContentTypeValueDefault = "event"; ///< default type value that identifies the type within the content array

public:
    using ArrayElementObject::ArrayElementObject;

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(std::string, type, std::string(ContentTypeValueDefault));   ///< determines our type in the JSON

    /// @brief Retrieve an element as a specific type
    template <typename T, std::enable_if_t<std::is_base_of_v<ContentObject, T>, int> = 0>
    [[nodiscard]] T get() const
    {
        return getTypedObject<T>();
    }

private:
    /// @brief Constructs an object of type `T` if its type matches the JSON type
    /// @throws std::invalid_argument if there is a type mismatch
    template <typename T, std::enable_if_t<std::is_base_of_v<ContentObject, T>, int> = 0>
    [[nodiscard]] T getTypedObject() const
    {
        if (type() != T::ContentTypeValue) {
            throw std::invalid_argument("Type mismatch: expected " + std::string(T::ContentTypeValue) +
                                        ", got " + type());
        }
        return T(root(), pointer());
    }

    friend class ContentArray;
};

/**
 * @class ContentArray
 * @brief Class for content arrays.
 *
 * Allows arrays of any type that derives from @ref ContentObject. An exampled of how
 * to get type instances is:
 * 
 * @code{.cpp}
 * auto next = content[index]; // gets the base ContentObject instance.
 * if (next.type() == layout::Group::ContentTypeValue) {
 *     auto group = next.get<layout::Group>(); // gets the instance typed as a layout::Group.
 *     // process group
 * } else if (next.type() == layout::Staff::ContentTypeValue) {
 *     auto staff = next.get<layout::Staff>(); // gets the instance typed as a layout::Staff.
 *     // process staff
 * }
 * @endcode
 *
 * To add instances to the array, use the template paramter to specify the type to add.
 *
 * @code{.cpp}
 * auto newElement = content.append<layout::Staff>();
 * @endcode
 *
 * The `append` method automatically gives the instance the correct `type` value.
 *
*/
class ContentArray : public Array<ContentObject>
{
public:
    using BaseArray = Array<ContentObject>;     ///< The base array type
    using BaseArray::BaseArray;  // Inherit constructors

    /// @brief Retrieve an element from the array as a specific type
    template <typename T, std::enable_if_t<std::is_base_of_v<ContentObject, T>, int> = 0>
    [[nodiscard]] T get(size_t index) const
    {
        this->checkIndex(index);
        return operator[](index).get<T>();
    }

    /// @brief Append an element of the specified type
    template <typename T, typename... Args,
              std::enable_if_t<std::is_base_of_v<ContentObject, T>, int> = 0>
    T append(Args&&... args)
    {
        auto result = BaseArray::append<T>(std::forward<Args>(args)...);
        if constexpr (T::ContentTypeValue != ContentObject::ContentTypeValueDefault) {
            result.set_type(std::string(T::ContentTypeValue));            
        }
        return result;
    }

private:
    /// @brief Constructs an object of type `T` if its type matches the JSON type
    /// @throws std::invalid_argument if there is a type mismatch
    template <typename T, std::enable_if_t<std::is_base_of_v<ContentObject, T>, int> = 0>
    [[nodiscard]] T getTypedObject(size_t index) const
    {
        this->checkIndex(index);
        auto element = (*this)[index];
        if (element.type() != T::ContentTypeValue) {
            throw std::invalid_argument("Type mismatch: expected " + std::string(T::ContentTypeValue) +
                                        ", got " + element.type());
        }
        return T(root(), pointer() / std::to_string(index));
    }
};

/**
 * @class EnumStringMapping
 * @brief Supplies enum string mappings to nlohmann json's serializer.
 */
template <typename E, typename = std::enable_if_t<std::is_enum_v<E>>>
struct EnumStringMapping
{
    static const std::unordered_map<std::string, E> stringToEnum();     ///< @brief maps strings to enum values

    /// @brief maps enum values to strings
    static const std::unordered_map<E, std::string> enumToString()
    {
        static const std::unordered_map<E, std::string> reverseMap = []() {
            std::unordered_map<E, std::string> result;
            for (const auto& element : EnumStringMapping<E>::stringToEnum()) {
                result.emplace(element.second, element.first);
            }
            return result;
        }();
        return reverseMap;
    }
};

/**
 * @brief Represents an MNX dictionary, where each key is a user-defined string.
 */
template <typename T>
class Dictionary : public Object
{
    static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, std::string> ||
                  std::is_base_of_v<ArrayElementObject, T>, "Invalid MNX dictionary element type.");

private:    
    template <typename DictionaryType, typename IteratorType>
    struct iter
    {
        using value_type = std::pair<const std::string, T>;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        using pointer = value_type*;
        using reference = value_type&;

    private:
        DictionaryType* m_ptr;
        IteratorType m_it;
        mutable std::unique_ptr<value_type> m_pair;  // Cached key-value pair for operator*() and operator->()

        void update_pair() const {
            m_pair.reset();
            if (m_it != m_ptr->ref().end()) {
                m_pair = std::make_unique<value_type>(m_it.key(), m_ptr->operator[](m_it.key()));
            }
        }
    
    public:
        iter(DictionaryType* ptr, IteratorType it) : m_ptr(ptr), m_it(it)
        { update_pair(); }

        [[nodiscard]] reference operator*() const { return *m_pair.get(); }
        [[nodiscard]] pointer operator->() const { return m_pair.get(); }

        iter& operator++() { ++m_it; update_pair(); return *this; }
        iter operator++(int) { iter tmp = *this; ++(*this); return tmp; }

        [[nodiscard]] bool operator!=(const iter& o) const { return m_it != o.m_it; }
        [[nodiscard]] bool operator==(const iter& o) const { return m_it == o.m_it; }
    };

public:
    /// @brief The type for elements in this Array.
    using value_type = T;

    using iterator = iter<Dictionary, json::iterator>;  ///< non-const iterator type
    using const_iterator = iter<const Dictionary, json::const_iterator>; ///< const iterator type

    /// @brief Wraps an Dictionary class around an existing JSON node
    /// @param root Reference to the document root
    /// @param pointer The json_pointer value for the node
    Dictionary(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Dictionary class as a child of a JSON node
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Dictionary(Base& parent, std::string_view key)
        : Object(parent, key) {}

    /** @brief Get the size of the array. */
    [[nodiscard]] size_t size() const { return ref().size(); }

    /** @brief Check if the array is empty. */
    [[nodiscard]] bool empty() const { return ref().empty(); }

    /** @brief Clear all elements. */
    void clear() { ref().clear(); }

    /** @brief Direct getter for a particular element. */
    [[nodiscard]] T at(size_t index) const
    {
        return operator[](index);
    }

    /// @brief const operator[]
    [[nodiscard]] auto operator[](const std::string& key) const
    {
        if constexpr (std::is_base_of_v<Base, T>) {
            return getChild<T>(key);
        } else {
            return getChild<SimpleType<T>>(key);
        }
    }

    /// @brief non-const operator[]
    [[nodiscard]] auto operator[](const std::string& key)
    {
        if constexpr (std::is_base_of_v<Base, T>) {
            return getChild<T>(key);
        } else {
            return getChild<SimpleType<T>>(key);
        }
    }

    /** @brief Add a new value to the dictonary. (Available only for primitive types) */
    template <typename U = T>
    std::enable_if_t<!std::is_base_of_v<Base, U>, void>
    emplace(const std::string& key, const U& value)
    {
        ref()[key] = value;
    }

    /**
     * @brief Create a new element using the input key. (Available only for Base types)
     * @return The newly created element.
    */
    template <typename U = T, typename... Args,
              std::enable_if_t<std::is_base_of_v<Base, U>, int> = 0>
    U append(const std::string& key, Args&&... args)
    {
        if constexpr (std::is_base_of_v<Object, U>) {
            ref()[key] = json::object();
        } else {
            ref()[key] = json::array();
        }
        return U(*this, key, std::forward<Args>(args)...);
    }

    /** @brief Remove an element at a given key. */
    void erase(const std::string& key)
    {
        ref().erase(key);
    }

    /// @brief Finds an element by key and returns an iterator.
    /// @param key The key to search for.
    /// @return Iterator to the found element or end() if not found.
    [[nodiscard]] auto find(const std::string& key)
    {
        auto it = ref().find(key);
        return (it != ref().end()) ? iterator(this, it) : end();
    }

    /// @brief Finds an element by key and returns a const iterator.
    /// @param key The key to search for.
    /// @return Const iterator to the found element or end() if not found.
    [[nodiscard]] auto find(const std::string& key) const
    {
        auto it = ref().find(key);
        return (it != ref().end()) ? const_iterator(this, it) : end();
    }

    /// @brief Returns true if the key exists in in the dictionary.
    /// @param key  The key to search for.
    [[nodiscard]] bool contains(const std::string& key) const
    { return find(key) != end(); }

    /// @brief Returns an iterator to the beginning of the dictionary.
    [[nodiscard]] auto begin() { return iterator(this, ref().begin()); }

    /// @brief Returns an iterator to the end of the dictionary.
    [[nodiscard]] auto end() { return iterator(this, ref().end()); }

    /// @brief Returns a const iterator to the beginning of the dictionary.
    [[nodiscard]] auto begin() const { return const_iterator(this, ref().begin()); }

    /// @brief Returns a const iterator to the end of the dictionary.
    [[nodiscard]] auto end() const { return const_iterator(this, ref().end()); }
};

} // namespace mnx

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

namespace nlohmann {

#if defined(_WIN32)
// This general adl_serializer is enabled only for enum types.
// For some reason MSC does not like the direct function definitions below.
template<typename EnumType>
struct adl_serializer<EnumType, std::enable_if_t<std::is_enum_v<EnumType>>>
{
    template<typename BasicJsonType>
    static EnumType from_json(const BasicJsonType& j)
    {
        // Lookup the string in the specialized map.
        const auto& map = ::mnx::EnumStringMapping<EnumType>::stringToEnum();
        auto it = map.find(j.get<std::string>());
        if (it != map.end()) {
            return it->second;
        }
        /// @todo throw or log unmapped string
        return EnumType{};
    }

    template<typename BasicJsonType>
    static void to_json(BasicJsonType& j, const EnumType& value)
    {
        const auto& map = ::mnx::EnumStringMapping<EnumType>::enumToString();
        auto it = map.find(value);
        if (it == map.end()) {
            /// @todo log or throw unmapped enum.
            j = BasicJsonType();
            return;
        }
        j = it->second;
    }
};
#else
// Clang works with the adl_specialization above, but GCC does not.
namespace detail {

template<typename BasicJsonType, typename EnumType,
         std::enable_if_t<std::is_enum<EnumType>::value, int> = 0>
inline void from_json(const BasicJsonType& j, EnumType& value)
{
    // Lookup the string in the specialized map.
    const auto& map = ::mnx::EnumStringMapping<EnumType>::stringToEnum();
    auto it = map.find(j.template get<std::string>());
    if (it != map.end()) {
        value = it->second;
    } else {
        /// @todo throw or log unmapped string
        value = EnumType{};
    }
}

template<typename BasicJsonType, typename EnumType,
         std::enable_if_t<std::is_enum<EnumType>::value, int> = 0>
inline void to_json(BasicJsonType& j, EnumType value) noexcept
{
    const auto& map = ::mnx::EnumStringMapping<EnumType>::enumToString();
    auto it = map.find(value);
    if (it != map.end()) {
        j = it->second;
    } else {
        /// @todo log or throw unmapped enum.
        j = BasicJsonType();
    }
}

} // namespace detail
#endif // defined(_WIN32)

} // namespace nlohmann

#endif // DOXYGEN_SHOULD_IGNORE_THIS
