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

#include "BaseTypes.h"

namespace mnx {

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
template <typename BaseContentT>
class ContentArray : public Array<BaseContentT>
{
public:
    using BaseArray = Array<BaseContentT>;     ///< The base array type
    using BaseArray::BaseArray;  // Inherit constructors

    /// @brief Retrieve an element from the array as a specific type
    template <typename T, std::enable_if_t<std::is_base_of_v<BaseContentT, T>, int> = 0>
    [[nodiscard]] T get(size_t index) const
    {
        this->checkIndex(index);
        return this->operator[](index).template get<T>();
    }

    /// @brief Append an element of the specified type (default overload for no-arg content types).
    template <typename T,
              std::enable_if_t<std::is_base_of_v<BaseContentT, T> &&
                               !std::is_same_v<T, sequence::Event> &&
                               !std::is_same_v<T, sequence::Space> &&
                               !std::is_same_v<T, sequence::MultiNoteTremolo> &&
                               !std::is_same_v<T, sequence::Tuplet>, int> = 0>
    T append()
    {
        return appendWithType<T>();
    }

    /// @brief Append overload entry point for explicitly specialized argful content types.
    template <typename T, typename... Args,
              std::enable_if_t<std::is_base_of_v<BaseContentT, T> && (sizeof...(Args) > 0), int> = 0>
    T append(const Args&... args)
    {
        static_assert(!std::is_same_v<T, T>,
                      "ContentArray::append requires explicit specialization for each content type.");
        return appendWithType<T>(args...);
    }

    // Prevent untemplated append() calls; callers must use append<T>(...).
    BaseContentT append(...) = delete;

protected:
    template <typename T, typename... Args>
    T appendWithType(Args&&... args)
    {
        static_assert(std::is_base_of_v<BaseContentT, T>,
                      "ContentArray::appendWithType requires a compatible content base type.");
        auto result = BaseArray::template append<T>(std::forward<Args>(args)...);
        const BaseContentT& contentObject = result;
        if (T::ContentTypeValue != contentObject.defaultType()) {
            result.set_type(std::string(T::ContentTypeValue));
        }
        return result;
    }

    /// @brief Constructs an object of type `T` if its type matches the JSON type
    /// @throws std::invalid_argument if there is a type mismatch
    template <typename T, std::enable_if_t<std::is_base_of_v<BaseContentT, T>, int> = 0>
    [[nodiscard]] T getTypedObject(size_t index) const
    {
        this->checkIndex(index);
        auto element = (*this)[index];
        if (element.type() != T::ContentTypeValue) {
            throw std::invalid_argument("Type mismatch: expected " + std::string(T::ContentTypeValue) +
                                        ", got " + element.type());
        }
        return T(this->root(), this->pointer() / std::to_string(index));
    }
};

} // namespace mnx
