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
 * @enum LayoutSymbol
 * @brief The symbols available to bracket a staff group
 */
enum class LayoutSymbol
{
    NoSymbol,       ///< the default (none)
    Brace,          ///< piano brace
    Bracket         ///< bracket
};

/**
 * @enum LabelRef
 * @brief The values available in a labelref
 */
enum class LabelRef
{
    Name,           ///< the full name from the part (the default)
    ShortName       ///< the abbreviated name from the part
};

/**
 * @enum LayoutStemDirection
 * @brief The values available in a labelref
 */
enum class LayoutStemDirection
{
    Down,           ///< stems down (default)
    Up              ///< stems up
};

/**
 * @class LayoutStaffSource
 * @brief Represents a system on a page in a score.
 */
class LayoutStaffSource : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;
    /// note that after adding a new ScorePage, the caller *must* provide a part id for it to validate

    MNX_OPTIONAL_PROPERTY(std::string, label);          ///< Text to appear to the left of the staff
    MNX_OPTIONAL_PROPERTY(LabelRef, labelref);          ///< The labelref to use (rather than label)
    MNX_REQUIRED_PROPERTY(std::string, part);           ///< Id of the part to draw from
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staff, 1);  ///< The staff number within the part.
    MNX_OPTIONAL_PROPERTY(LayoutStemDirection, stem);   ///< The stem direction for this staff source.
    MNX_OPTIONAL_PROPERTY(std::string, voice);          ///< The voice id to draw from.
};

/**
 * @class LayoutStaff
 * @brief Represents a single global measure instance within an MNX document.
 */
class LayoutStaff : public ContentObject
{
public:
    using ContentObject::ContentObject;

    /// @brief Creates a new Global class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    LayoutStaff(Base& parent, const std::string_view& key)
        : ContentObject(parent, key)
    {
        // required children
        create_sources();
    }

    MNX_OPTIONAL_PROPERTY(std::string, label);                  ///< Label to be rendered to the left of the staff
    MNX_OPTIONAL_PROPERTY(LabelRef, labelref);                  ///< The labelref to use (rather than label)
    MNX_REQUIRED_CHILD(Array<LayoutStaffSource>, sources);      ///< The sources for this staff.
    MNX_OPTIONAL_PROPERTY(LayoutSymbol, symbol);                ///< The symbol down the left side.

    static constexpr std::string_view ContentTypeValue = "staff";
};

/**
 * @class LayoutGroup
 * @brief Represents a single global measure instance within an MNX document.
 */
class LayoutGroup : public ContentObject
{
public:
    using ContentObject::ContentObject;

    /// @brief Creates a new Global class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    LayoutGroup(Base& parent, const std::string_view& key)
        : ContentObject(parent, key)
    {
        // required children
        create_content();
    }

    MNX_REQUIRED_CHILD(ContentArray, content);      ///< Required child containing the layout content (groups and staves).
    MNX_OPTIONAL_PROPERTY(std::string, label);      ///< Label to be rendered to the left of the group
    MNX_OPTIONAL_PROPERTY(LayoutSymbol, symbol);    ///< The symbol down the left side.

    static constexpr std::string_view ContentTypeValue = "group";
};

/**
 * @class Layout
 * @brief Represents the element of the layout array in an MNX document.
 */
class Layout : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    /// @brief Creates a new Global class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    Layout(Base& parent, const std::string_view& key)
        : ArrayElementObject(parent, key)
    {
        // required children
        create_content();
    }

    MNX_REQUIRED_CHILD(ContentArray, content);      ///< Required child containing the layout content (groups and staves).
    MNX_REQUIRED_PROPERTY(std::string, id);         ///< The unique identifier used to identify this layout.
};

} // namespace mnx