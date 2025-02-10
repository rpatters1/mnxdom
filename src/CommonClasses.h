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

class Fraction : private Array<unsigned int>
{
private:
    using NumType = unsigned int;
    using ArrayType = Array<NumType>;

    static constexpr size_t NUMERATOR_INDEX = 0;
    static constexpr size_t DENOMINATOR_INDEX = 1;

public:
    Fraction(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayType(root, pointer)
    {
        if (size() != 2) {
            throw std::invalid_argument("mnx::Fraction must have exactly 2 elements.");
        }
    }

    /// @brief Creates a new Array class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param numerator The numerator (number on top) of the fraction.
    /// @param denominator The denominator (number on bottom) of the fraction.
    Fraction(Base& parent, const std::string_view& key, NumType numerator, NumType denominator)
        : ArrayType(parent, key)
    {
        push_back(numerator);
        push_back(denominator);
    }

    MNX_ARRAY_ELEMENT_PROPERTY(NumType, numerator, NUMERATOR_INDEX);        ///< the numerator of the fraction
    MNX_ARRAY_ELEMENT_PROPERTY(NumType, denominator, DENOMINATOR_INDEX);    ///< the denominator of the fraction

    friend class Base;
};

/**
 * @class RythmicPosition
 * @brief Represents a system on a page in a score.
 */
class RythmicPosition : public Object
{
public:
    /// @brief Constructor for existing rhythmic position instances
    RythmicPosition(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }
    
    /// @brief Creates a new RythmicPosition class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param numerator The numerator (number on top) of the fraction.
    /// @param denominator The denominator (number on bottom) of the fraction.
    RythmicPosition(Base& parent, const std::string_view& key, unsigned int numerator, unsigned int denominator)
        : Object(parent, key)
    {
        create_fraction(numerator, denominator);
    }

    MNX_REQUIRED_CHILD(Fraction, fraction);             ///< The metric position, where 1/4 is a quarter note.
    MNX_OPTIONAL_PROPERTY(unsigned int, graceIndex);    ///< The grace note index of this position.
                                                        ///< (0 is the primary, and then count to the left.)
};

/**
 * @class MeasureRythmicPosition
 * @brief Represents a system on a page in a score.
 */
class MeasureRythmicPosition : public Object
{
public:
    /// @brief Constructor for existing rhythmic position instances
    MeasureRythmicPosition(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }
    
    /// @brief Creates a new MeasureRythmicPosition class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param measureId The measure index of the measure of the position.
    /// @param numerator The numerator (number on top) of the fraction.
    /// @param denominator The denominator (number on bottom) of the fraction.
    MeasureRythmicPosition(Base& parent, const std::string_view& key, int measureId, unsigned int numerator, unsigned int denominator)
        : Object(parent, key)
    {
        set_bar(measureId);
        create_position(numerator, denominator);
    }

    MNX_REQUIRED_PROPERTY(int, bar);                ///< The measure id of the measure of this MeasureRythmicPosition.
    MNX_REQUIRED_CHILD(RythmicPosition, position);  ///< The metric position, where 1/4 is a quarter note.
};
} // namespace mnx
