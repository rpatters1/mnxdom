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
#include <numeric>
#include <stdexcept>
#include <limits>

#include "BaseTypes.h"
#include "Enumerations.h"

namespace mnx {

/**
 * @class FractionValue
 * @brief Represents a detached arithmetic fraction with normalization.
 *
 * This class provides arithmetic operations on rational numbers without
 * attaching them to the JSON DOM. It is intended for musical-duration and
 * proportional calculations where a simple fraction is needed independently
 * of the MNX DOM structure.
 *
 * A FractionValue constructed from a numerator and denominator preserves the
 * values exactly as provided. Arithmetic operations (+=, -=, *=, /=) normalize
 * the result to lowest terms using std::gcd. Call normalize() explicitly if
 * you need a reduced form immediately after construction.
 *
 * The class supports:
 *  - Exact rational arithmetic (addition, subtraction, multiplication, division)
 *  - Normalization to lowest terms after each operation
 *  - Comparison operators (in helper section)
 *
 * Compound assignment operators (+=, -=, *=, /=) are implemented as members.
 * Binary operators (+, -, *, /) are implemented as non-member functions to
 * allow symmetric conversions and correct C++ arithmetic semantics.
 */
struct [[nodiscard]] FractionValue
{
    /// @brief Unsigned integer type used for numerator and denominator.
    using NumType = unsigned;

private:
    NumType m_num = 0; ///< Numerator of the fraction (always non-negative).
    NumType m_den = 1; ///< Denominator of the fraction (must be > 0).

public:
    /**
     * @brief Default constructor initializes the value to 0/1.
     */
    FractionValue() = default;

    /**
     * @brief Constructs a fraction from a numerator and denominator.
     *
     * @param num The numerator.
     * @param den The denominator. Must not be zero.
     *
     * @throws std::invalid_argument if @p den is zero.
     *
     * The fraction is not automatically reduced. Use normalize() if you need
     * the value in lowest terms.
     */
    FractionValue(NumType num, NumType den)
        : m_num(num)
        , m_den(den)
    {
        if (m_den == 0) {
            throw std::invalid_argument("FractionValue: denominator must not be zero.");
        }
    }

    /**
     * @brief Constructs a Fraction object from an integer.
     * @param value The integer value of the fraction.
     * @throws std::invalid_argument if the m_denominator is zero.
     */
    constexpr FractionValue(int value) : m_num(value), m_den(1) {}

    /// @brief Returns the numerator.
    constexpr NumType numerator() const noexcept   { return m_num; }

    /// @brief Returns the denominator.
    constexpr NumType denominator() const noexcept { return m_den; }

    /// @brief Constructs the max fractional value.
    static constexpr FractionValue max() noexcept
    {
        return FractionValue((std::numeric_limits<NumType>::max)());
    }

    /**
     * @brief Adds another FractionValue to this one.
     *
     * @param rhs The fraction to add.
     * @return Reference to this object after modification.
     *
     * The result is normalized.
     */
    FractionValue& operator+=(const FractionValue& rhs)
    {
        // a/b + c/d = (ad + bc)/bd
        m_num = m_num * rhs.m_den + rhs.m_num * m_den;
        m_den = m_den * rhs.m_den;
        normalize();
        return *this;
    }

    /**
     * @brief Subtracts another FractionValue from this one.
     *
     * @param rhs The fraction to subtract.
     * @return Reference to this object after modification.
     *
     * The result is normalized.
     */
    constexpr FractionValue& operator-=(const FractionValue& rhs)
    {
        // a/b - c/d = (ad - bc)/bd
        m_num = m_num * rhs.m_den - rhs.m_num * m_den;
        m_den = m_den * rhs.m_den;
        normalize();
        return *this;
    }

    /**
     * @brief Multiplies this fraction by another.
     *
     * @param rhs The fraction to multiply with.
     * @return Reference to this object after modification.
     *
     * The result is normalized.
     */
    constexpr FractionValue& operator*=(const FractionValue& rhs)
    {
        m_num = m_num * rhs.m_num;
        m_den = m_den * rhs.m_den;
        normalize();
        return *this;
    }

    /**
     * @brief Divides this fraction by another.
     *
     * @param rhs The divisor.
     * @return Reference to this object after modification.
     *
     * @throws std::invalid_argument if @p rhs has a numerator of zero.
     *
     * The result is normalized.
     */
    FractionValue& operator/=(const FractionValue& rhs)
    {
        if (rhs.m_num == 0) {
            throw std::invalid_argument("Division by zero FractionValue.");
        }
        m_num = m_num * rhs.m_den;
        m_den = m_den * rhs.m_num;
        normalize();
        return *this;
    }

    /**
     * @brief Reduces the fraction to lowest terms using std::gcd.
     */
    constexpr void normalize()
    {
        const NumType g = std::gcd(m_num, m_den);
        if (g > 1) {
            m_num /= g;
            m_den /= g;
        }
    }
};

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

// ------------------------------------------------------------
// Non-member arithmetic operators
// ------------------------------------------------------------

constexpr FractionValue operator+(FractionValue lhs, const FractionValue& rhs)
{
    lhs += rhs;
    return lhs;
}

constexpr FractionValue operator-(FractionValue lhs, const FractionValue& rhs)
{
    lhs -= rhs;
    return lhs;
}

constexpr FractionValue operator*(FractionValue lhs, const FractionValue& rhs)
{
    lhs *= rhs;
    return lhs;
}

inline FractionValue operator/(FractionValue lhs, const FractionValue& rhs)
{
    lhs /= rhs;
    return lhs;
}

// ------------------------------------------------------------
// Comparison operators
// ------------------------------------------------------------

constexpr bool operator==(const FractionValue& a, const FractionValue& b)
{
    return a.numerator() * b.denominator() == b.numerator() * a.denominator();
}

constexpr bool operator!=(const FractionValue& a, const FractionValue& b)
{
    return !(a == b);
}

constexpr bool operator<(const FractionValue& a, const FractionValue& b)
{
    return a.numerator() * b.denominator() < b.numerator() * a.denominator();
}

constexpr bool operator<=(const FractionValue& a, const FractionValue& b)
{
    return !(b < a);
}

constexpr bool operator>(const FractionValue& a, const FractionValue& b)
{
    return b < a;
}

constexpr bool operator>=(const FractionValue& a, const FractionValue& b)
{
    return !(a < b);
}

#endif // DOXYGEN_SHOULD_IGNORE_THIS

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
    /// @param key The JSON key to use for embedding in parent.
    /// @param value The numerator (number on top) and denominator (number on bottom) of the fraction.
    Fraction(Base& parent, const std::string_view& key, const FractionValue& value)
        : ArrayType(parent, key)
    {
        push_back(value.numerator());
        push_back(value.denominator());
    }

    /// @brief Implicit conversion to @ref FractionValue for arithmetic and comparisons.
    operator FractionValue() const
    {
        return FractionValue(numerator(), denominator());
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
    /// @param key The JSON key to use for embedding in parent.
    /// @param position Position within a measure (as a fraction of whole notes)
    RhythmicPosition(Base& parent, const std::string_view& key, const FractionValue& position)
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
    MeasureRhythmicPosition(Base& parent, const std::string_view& key, int measureId, const FractionValue& position)
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
    /// @param staffDistance The number of diatonic steps in the interval (negative is down)
    /// @param halfSteps The number of 12-EDO chromatic halfsteps in the interval (negative is down)
    Interval(Base& parent, const std::string_view& key, int staffDistance, int halfSteps)
        : Object(parent, key)
    {
        set_halfSteps(halfSteps);
        set_staffDistance(staffDistance);
    }

    MNX_REQUIRED_PROPERTY(int, halfSteps);      ///< the number of 12-EDO chromatic halfsteps in the interval (negative is down)
    MNX_REQUIRED_PROPERTY(int, staffDistance);  ///< the number of diatonic steps in the interval (negative is down)
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
    /// @brief initializer class for #NoteValue
    class Initializer
    {
    public:
        NoteValueBase base;     ///< the note value base to initialize
        unsigned dots;          ///< the number of dots to initialize

        /// @brief constructor
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

    /// @brief Convert the note value to a Fraction base where a quarter note is 1/4.
    operator FractionValue() const;
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


    /// @brief Convert the note value quantity to a Fraction base where a quarter note is 1/4.
    operator FractionValue() const
    { return multiple() * duration(); }
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

    /// @brief Implicit converter to FractionValue.
    operator FractionValue() const
    { return count() * FractionValue(1, static_cast<unsigned>(unit())); }
};

} // namespace mnx
