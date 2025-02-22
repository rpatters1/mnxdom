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

/// @class Fraction
/// @brief Represents a fraction of a whole note, for measuring musical time.
class Fraction : private Array<unsigned>
{
private:
    using NumType = unsigned;
    using ArrayType = Array<NumType>;

    static constexpr size_t NUMERATOR_INDEX = 0;
    static constexpr size_t DENOMINATOR_INDEX = 1;

public:
    /// @brief Constructor to wrap a Fraction instance around existing JSON
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
 * @class RhythmicPosition
 * @brief Represents a system on a page in a score.
 */
class RhythmicPosition : public Object
{
public:
    /// @brief Constructor for existing rhythmic position instances
    RhythmicPosition(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }
    
    /// @brief Creates a new RhythmicPosition class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param numerator The numerator (number on top) of the fraction.
    /// @param denominator The denominator (number on bottom) of the fraction.
    RhythmicPosition(Base& parent, const std::string_view& key, unsigned numerator, unsigned denominator)
        : Object(parent, key)
    {
        create_fraction(numerator, denominator);
    }

    MNX_REQUIRED_CHILD(Fraction, fraction);             ///< The metric position, where 1/4 is a quarter note.
    MNX_OPTIONAL_PROPERTY(unsigned, graceIndex);    ///< The grace note index of this position.
                                                        ///< (0 is the primary, and then count to the left.)
};

/**
 * @class MeasureRhythmicPosition
 * @brief Represents a system on a page in a score.
 */
class MeasureRhythmicPosition : public Object
{
public:
    /// @brief Constructor for existing rhythmic position instances
    MeasureRhythmicPosition(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }
    
    /// @brief Creates a new MeasureRhythmicPosition class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param measureId The measure index of the measure of the position.
    /// @param numerator The numerator (number on top) of the fraction.
    /// @param denominator The denominator (number on bottom) of the fraction.
    MeasureRhythmicPosition(Base& parent, const std::string_view& key, int measureId, unsigned numerator, unsigned denominator)
        : Object(parent, key)
    {
        set_measure(measureId);
        create_position(numerator, denominator);
    }

    MNX_REQUIRED_PROPERTY(int, measure);            ///< The measure id of the measure of this MeasureRhythmicPosition.
    MNX_REQUIRED_CHILD(RhythmicPosition, position); ///< The metric position, where 1/4 is a quarter note.
};

/**
 * @class KeySignature
 * @brief Represents a key signature
 */
class KeySignature : public Object
{
public:
    /// @brief Constructor for existing KeySignature objects
    KeySignature(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new KeySignature class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param fifths The number of fifths distance from a signature with no accidentals.
    KeySignature(Base& parent, const std::string_view& key, int fifths)
        : Object(parent, key)
    {
        set_fifths(fifths);
    }

    MNX_OPTIONAL_NAMED_PROPERTY(std::string, styleClass, "class"); ///< style class
    MNX_OPTIONAL_PROPERTY(std::string, color);                  ///< color to use when rendering the key signature
    MNX_REQUIRED_PROPERTY(int, fifths);                         ///< offset from signature with no accidentals
};

/**
 * @class NoteValue
 * @brief Represents a symbolic note value (not necessarily a duration)
 */
class NoteValue : public Object
{
public:
    /// @brief Constructor for existing NoteValue instances
    NoteValue(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Barline class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param noteValueBase The note value base for this Barline
    /// @param inpDots The number of dots, if any 
    NoteValue(Base& parent, const std::string_view& key, NoteValueBase noteValueBase, unsigned inpDots = 0)
        : Object(parent, key)
    {
        set_base(noteValueBase);
        if (inpDots) {
            set_dots(inpDots);
        }
    }

    MNX_REQUIRED_PROPERTY(NoteValueBase, base);                 ///< the type ("base") of note
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(unsigned, dots, 0);      ///< the number of dots
};

/**
 * @class NoteValueQuantity
 * @brief Represents a quantity of symbolic note values=
 */
class NoteValueQuantity : public Object
{
public:
    /// @brief Constructor for existing NoteValue instances
    NoteValueQuantity(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Barline class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param count The quantity of note value units
    /// @param noteValueBase The note value units
    /// @param dots The number of dots, if any 
    NoteValueQuantity(Base& parent, const std::string_view& key, unsigned count, NoteValueBase noteValueBase, unsigned dots = 0)
        : Object(parent, key)
    {
        set_multiple(count);
        create_duration(noteValueBase, dots);
    }

    MNX_REQUIRED_CHILD(NoteValue, duration);                    ///< duration unit
    MNX_REQUIRED_PROPERTY(unsigned, multiple);                  ///< quantity of duration units
};

/**
 * @class TimeSignature
 * @brief Represents the tempo for a global measure.
 */
class TimeSignature : public Object
{
public:
    /// @brief Constructor for existing NoteValue instances
    TimeSignature(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Barline class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param count The number of beats per minutes
    /// @param unit The note value base for this Barline
    TimeSignature(Base& parent, const std::string_view& key, int count, TimeSignatureUnit unit)
        : Object(parent, key)
    {
        set_count(count);
        set_unit(unit);
    }

    MNX_REQUIRED_PROPERTY(int, count);                ///< the number of beats (top number)
    MNX_REQUIRED_PROPERTY(TimeSignatureUnit, unit);   ///< the unit value (bottom number)
};

} // namespace mnx
