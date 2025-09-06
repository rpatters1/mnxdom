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

#include "Enumerations.h"
#include "BaseTypes.h"

namespace mnx {

/**
 * @namespace mnx::layout
 * @brief classes related to the root layouts array
 */
namespace layout {

/**
* @class StaffSource
 * @brief Represents a staff source for a staff in a layout in a score.
 */
class StaffSource : public ArrayElementObject
{
public:
    /// @brief Constructor for existing staff sources
    StaffSource(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }
    
    /// @brief Creates a new StaffSource class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param partId The part to use as a source
    StaffSource(Base& parent, const std::string_view& key, std::string& partId)
        : ArrayElementObject(parent, key)
    {
        set_part(partId);
    }

    MNX_OPTIONAL_PROPERTY(std::string, label);          ///< Text to appear to the left of the staff
    MNX_OPTIONAL_PROPERTY(LabelRef, labelref);          ///< The labelref to use (rather than label)
    MNX_REQUIRED_PROPERTY(std::string, part);           ///< Id of the part to draw from
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staff, 1);  ///< The staff number within the part.
    MNX_OPTIONAL_PROPERTY(StemDirection, stem);   ///< The stem direction for this staff source.
    MNX_OPTIONAL_PROPERTY(std::string, voice);          ///< The voice id to draw from.
};

/**
 * @class Staff
 * @brief Represents a single staff instance within an MNX layout.
 */
class Staff : public ContentObject
{
public:
    using ContentObject::ContentObject;

    /// @brief Creates a new Staff class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Staff(Base& parent, const std::string_view& key)
        : ContentObject(parent, key)
    {
        // required children
        create_sources();
    }

    MNX_OPTIONAL_PROPERTY(std::string, label);                  ///< Label to be rendered to the left of the staff
    MNX_OPTIONAL_PROPERTY(LabelRef, labelref);                  ///< The labelref to use (rather than label)
    MNX_REQUIRED_CHILD(Array<StaffSource>, sources);      ///< The sources for this staff.
    MNX_OPTIONAL_PROPERTY(LayoutSymbol, symbol);                ///< The symbol down the left side.

    inline static constexpr std::string_view ContentTypeValue = "staff"; ///< type value that identifies the type within the content array
};

/**
 * @class Group
 * @brief Represents a groups of staves within an MNX layout.
 */
class Group : public ContentObject
{
public:
    using ContentObject::ContentObject;

    /// @brief Creates a new Group class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Group(Base& parent, const std::string_view& key)
        : ContentObject(parent, key)
    {
        // required children
        create_content();
    }

    MNX_REQUIRED_CHILD(ContentArray, content);      ///< Required child containing the layout content (groups and staves).
    MNX_OPTIONAL_PROPERTY(std::string, label);      ///< Label to be rendered to the left of the group
    MNX_OPTIONAL_PROPERTY(LayoutSymbol, symbol);    ///< The symbol down the left side.

    inline static constexpr std::string_view ContentTypeValue = "group"; ///< type value that identifies the type within the content array
};

} // namespace layout

/**
 * @class Layout
 * @brief Represents the element of the layout array in an MNX document.
 */
class Layout : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    /// @brief Creates a new Layout class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Layout(Base& parent, const std::string_view& key)
        : ArrayElementObject(parent, key)
    {
        // required children
        create_content();
    }

    MNX_REQUIRED_CHILD(ContentArray, content);      ///< Required child containing the layout content (groups and staves).

    inline static constexpr std::string_view JsonSchemaTypeName = "system-layout";     ///< required for mapping
};

} // namespace mnx