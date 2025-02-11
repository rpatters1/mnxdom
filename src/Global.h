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
 * @namespace mnx::global
 * @brief classes related to the root global object
 */
namespace global {

/**
 * @class Barline
 * @brief Represents the barline for a global measure.
 */
class Barline : public Object
{
public:
    /// @brief Constructor for existing global barlines
    Barline(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Barline class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param barlineType The barline type for this Barline
    Barline(Base& parent, const std::string_view& key, BarlineType barlineType)
        : Object(parent, key)
    {
        set_type(barlineType);
    }

    MNX_REQUIRED_PROPERTY(BarlineType, type);  ///< the type of barline
};

/**
 * @class Ending
 * @brief Represents an alternate ending (or "volta bracket") for a global measure.
 */
class Ending : public Object
{
public:
    /// @brief Constructor for existing global barlines
    Ending(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Ending class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param duration The duration of th ending
    Ending(Base& parent, const std::string_view& key, int duration)
        : Object(parent, key)
    {
        set_duration(duration);
    }

    MNX_OPTIONAL_NAMED_PROPERTY(std::string, styleClass, "class"); ///< style class
    MNX_OPTIONAL_PROPERTY(std::string, color);      ///< color to use when rendering the ending
    MNX_REQUIRED_PROPERTY(int, duration);           ///< the type of barline
    MNX_OPTIONAL_CHILD(Array<int>, numbers);        ///< ending numbers
    MNX_OPTIONAL_PROPERTY(bool, open);              ///< if this is an open (i.e., final) ending
};

/**
 * @class Measure
 * @brief Represents a single global measure instance within an MNX document.
 */
class Measure : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    MNX_OPTIONAL_CHILD(Barline, barline);     ///< the barline for this measure
    MNX_OPTIONAL_PROPERTY(int, index);              ///< the measure index

    /// @brief Calculates the barline type for this measure.
    /// @return barline().type() if barline() has a value. Otherwise the default (as defined in the MNX specification.)
    BarlineType calcBarlineType() const;

    /// @brief Calculates the meausure index for this measure.
    /// @return index() if it has a value or the default value (defined in the MNX specification) if it does not.
    int calcMeasureIndex() const;
};

} // namespace global

/**
 * @class Global
 * @brief Represents the global section of an MNX document, containing global measures.
 */
class Global : public Object
{
public:
    using Object::Object;

    /// @brief Creates a new Global class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    Global(Base& parent, const std::string_view& key)
        : Object(parent, key)
    {
        // required children
        create_measures();
    }

    MNX_REQUIRED_CHILD(Array<global::Measure>, measures);     ///< array of global measures.
};

} // namespace mnx