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

class Document; // Forward declaration
/** @brief Shared `Document` pointer */
using DocumentPtr = std::shared_ptr<Document>;
/** @brief Shared weak `Document` pointer */
using DocumentWeakPtr = std::weak_ptr<Document>;

/**
 * @brief Base class for all MNX elements, ensuring a reference to its JSON node.
 */
class Base
{
public:
    virtual ~Base() = default;

    /**
     * @brief Convert this element for retrieval.
     * @return A reference to the JSON node.
     */
    const json& to_json() const { return m_json; }

    /**
     * @brief Access the JSON node for modification.
     * @return A reference to the JSON node.
     */
    json& to_json() { return m_json; }

    /**
     * @brief Gets a pointer to the Document.
     * @return A pointer to the Document instance.
     */
    DocumentPtr getDocument() const
    {
        auto document = m_document.lock();
        assert(document); // program bug if this pointer goes out of scope.
        return document;
    }

protected:
    /**
     * @brief Construct a Base element tied to a specific JSON node.
     * @param doc The containing Document.
     * @param json_ref The JSON node representing this element.
     */
    Base(const DocumentWeakPtr& doc, json& json_ref)
        : m_document(doc), m_json(json_ref) {}

private:
    DocumentWeakPtr m_document;
    json& m_json;

    friend class Document;
};

/**
 * @brief Macro to define a required property.
 */
#define MNX_REQUIRED_PROPERTY(Type, Name, JsonKey) \
    Type Name() const { return to_json().at(JsonKey).get<Type>(); } \
    void set_##Name(const Type& value) { to_json()[JsonKey] = value; }

/**
 * @brief Macro to define an optional property.
 */
#define MNX_OPTIONAL_PROPERTY(Type, Name, JsonKey) \
    std::optional<Type> Name() const { \
        if (to_json().contains(JsonKey)) return to_json().at(JsonKey).get<Type>(); \
        return std::nullopt; \
    } \
    void set_##Name(const Type& value) { to_json()[JsonKey] = value; } \
    void clear_##Name() { to_json().erase(JsonKey); }

/**
 * @brief Represents an MNX object, encapsulating property access.
 */
class Object : public Base
{
public:
    Object(DocumentWeakPtr doc, json& json_ref)
        : Base(doc, json_ref)
    {
        if (!json_ref.is_object()) {
            throw std::invalid_argument("mnx::Object must wrap a JSON object.")
        }
    }
};

/**
 * @brief Represents an MNX array, encapsulating property access.
 */
template <class T>
class Array : public Base
{
public:
    Array(DocumentWeakPtr doc, json& json_ref)
        : Base(doc, json_ref)
    {
        if (!json_ref.is_array()) {
            throw std::invalid_argument("mnx::Array must wrap a JSON object.")
        }
    }

    /** @brief Get the size of the array. */
    size_t size() const { return to_json().size(); }

    /** @brief Check if the array is empty. */
    bool empty() const { return to_json().empty(); }

    /** @brief Clear all elements. */
    void clear() { to_json().clear(); }

    /** @brief Access element at a given index. */
    const T& operator[](size_t index) const
    {
        assert(index < to_json().size());
        return to_json().at(index).get<T>();
    }

    /** @brief Modify element at a given index. */
    T& operator[](size_t index)
    {
        assert(index < to_json().size());
        return to_json().at(index).get_ref<T&>();
    }

    /** @brief Append a new value to the array. */
    void push_back(const T& value)
    {
        to_json().push_back(value);
    }

    /** @brief Remove an element at a given index. */
    void erase(size_t index) {
        assert(index < to_json().size());
        to_json().erase(to_json().begin() + index);
    }
};

/**
 * @brief Represents the root of an MNX document.
 */
class Document : public Object, public std::enable_shared_from_this<Document>
{
public:
    static DocumentPtr create() {
        return std::shared_ptr<Document>(new Document());
    }

private:
    Document() : Object(DocumentWeakPtr(), m_json_root)
    {
        m_document = shared_from_this();
    }

    json m_json_root;
};

} // namespace mnx
