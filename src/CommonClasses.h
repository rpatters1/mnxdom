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

#include <utility>

#include "BaseTypes.h"
#include "Enumerations.h"

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
    using Initializer = std::pair<NumType, NumType>;    ///< initializer for Fraction class (numerator, denominator)
    
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
    /// @param key The JSON key to use for embedding in parent.
    /// @param value The numerator (number on top) and denominator (number on bottom) of the fraction.
    Fraction(Base& parent, const std::string_view& key, const Initializer& value)
        : ArrayType(parent, key)
    {
        push_back(value.first);
        push_back(value.second);
    }

    MNX_ARRAY_ELEMENT_PROPERTY(NumType, numerator, NUMERATOR_INDEX);        ///< the numerator of the fraction
    MNX_ARRAY_ELEMENT_PROPERTY(NumType, denominator, DENOMINATOR_INDEX);    ///< the denominator of the fraction

    friend class Base;
};

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

inline bool operator==(const Fraction& a, const Fraction& b)
{
    return a.numerator() * b.denominator() == b.numerator() * a.denominator();
}

inline bool operator!=(const Fraction& a, const Fraction& b) {
    return !(a == b);
}

inline bool operator<(const Fraction& a, const Fraction& b) {
    return a.numerator() * b.denominator() < b.numerator() * a.denominator();
}

inline bool operator<=(const Fraction& a, const Fraction& b) {
    return !(b < a);
}

inline bool operator>(const Fraction& a, const Fraction& b) {
    return b < a;
}

inline bool operator>=(const Fraction& a, const Fraction& b) {
    return !(a < b);
}

#endif // DOXYGEN_SHOULD_IGNORE_THIS

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
    /// @param key The JSON key to use for embedding in parent.
    /// @param position Position within a measure (as a fraction of whole notes)
    RhythmicPosition(Base& parent, const std::string_view& key, const Fraction::Initializer& position)
        : Object(parent, key)
    {
        create_fraction(position);
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
    /// @param key The JSON key to use for embedding in parent.
    /// @param measureId The measure index of the measure of the position.
    /// @param position Position within the measure (as a fraction of whole notes)
    MeasureRhythmicPosition(Base& parent, const std::string_view& key, int measureId, const Fraction::Initializer& position)
        : Object(parent, key)
    {
        set_measure(measureId);
        create_position(position);
    }

    MNX_REQUIRED_PROPERTY(int, measure);            ///< The measure id of the measure of this MeasureRhythmicPosition.
    MNX_REQUIRED_CHILD(RhythmicPosition, position); ///< The metric position, where 1/4 is a quarter note.
};

/**
 * @class Interval
 * @brief Represents a musical chromatic interval
 */
class Interval : public Object
{
public:
    /// @brief Constructor for existing NoteValue instances
    Interval(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Barline class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param diatonic The number of diatonic steps in the interval (negative is down)
    /// @param chromatic The number of 12-EDO chromatic halfsteps in the interval (negative is down)
    Interval(Base& parent, const std::string_view& key, int diatonic, int chromatic)
        : Object(parent, key)
    {
        set_chromatic(chromatic);
        set_diatonic(diatonic);
    }

    MNX_REQUIRED_PROPERTY(int, chromatic);      ///< the number of 12-EDO chromatic halfsteps in the interval (negative is down)
    MNX_REQUIRED_PROPERTY(int, diatonic);       ///< the number of diatonic steps in the interval (negative is down)
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
    /// @param key The JSON key to use for embedding in parent.
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
    class Initializer
    {
    public:
        NoteValueBase base;
        unsigned dots;

        Initializer(NoteValueBase inBase, unsigned inDots = 0) : base(inBase), dots(inDots) {}
    };

    /// @brief Constructor for existing NoteValue instances
    NoteValue(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Barline class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param noteValue The note value
    NoteValue(Base& parent, const std::string_view& key, const Initializer& noteValue)
        : Object(parent, key)
    {
        set_base(noteValue.base);
        if (noteValue.dots) {
            set_dots(noteValue.dots);
        }
    }

    MNX_REQUIRED_PROPERTY(NoteValueBase, base);                 ///< the type ("base") of note
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(unsigned, dots, 0);      ///< the number of dots

    /// @brief Calculates the number of flags or beams required by this note value
    unsigned calcNumberOfFlags() const;
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
    /// @param key The JSON key to use for embedding in parent.
    /// @param count The quantity of note value units
    /// @param noteValue The note value
    NoteValueQuantity(Base& parent, const std::string_view& key, unsigned count, const NoteValue::Initializer& noteValue)
        : Object(parent, key)
    {
        set_multiple(count);
        create_duration(noteValue);
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
    /// @param key The JSON key to use for embedding in parent.
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
