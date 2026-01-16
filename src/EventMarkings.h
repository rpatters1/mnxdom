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
#include "Enumerations.h"

namespace mnx {
namespace sequence {

/**
 * @class EventMarkingBase
 * @brief Base class for event markings
 */
class EventMarkingBase : public Object
{
public:
    using Object::Object;
};

/**
 * @class Accent
 * @brief Class that represents an accent marking on an event
 */
class Accent : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;

    MNX_OPTIONAL_PROPERTY(MarkingUpDown, pointing);     ///< Specifies if the accent points upward or downward.
};

/**
 * @class BreathMark
 * @brief Class that represents a spiccato marking on an event
 */
class BreathMark : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;

    MNX_OPTIONAL_PROPERTY(BreathMarkSymbol, symbol);    ///< The symbol for the breath mark.
};

/**
 * @class SoftAccent
 * @brief Class that represents a soft accent marking on an event
 */
class SoftAccent : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;
};

/**
 * @class Spiccato
 * @brief Class that represents a spiccato marking on an event
 */
class Spiccato : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;
};

/**
 * @class Staccatissimo
 * @brief Class that represents a staccatissimo marking on an event
 */
class Staccatissimo : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;
};

/**
 * @class Staccato
 * @brief Class that represents a staccato marking on an event
 */
class Staccato : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;
};

/**
 * @class Stress
 * @brief Class that represents a stress marking on an event
 */
class Stress : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;
};

/**
 * @class StrongAccent
 * @brief Class that represents a strong accent marking on an event
 */
class StrongAccent : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;

    MNX_OPTIONAL_PROPERTY(MarkingUpDown, pointing);     ///< Specifies if the accent points upward or downward.
};

/**
 * @class Tenuto
 * @brief Class that represents a tenuto marking on an event
 */
class Tenuto : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;
};

/**
 * @class SingleNoteTremolo
 * @brief Class that represents single-note tremolo marking on an event
 */
class SingleNoteTremolo : public EventMarkingBase
{
public:
    /// @brief initializer class for #SingleNoteTremolo
    struct Fields
    {
        unsigned marks{}; ///< the number of marks
    };

    /// @brief Constructor for existing SingleNoteTremolo objects
    SingleNoteTremolo(const std::shared_ptr<json>& root, json_pointer pointer)
        : EventMarkingBase(root, pointer)
    {
    }

    /// @brief Creates a new SingleNoteTremolo class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The tremolo fields to use.
    SingleNoteTremolo(Base& parent, std::string_view key, const Fields& fields)
        : EventMarkingBase(parent, key)
    {
        set_marks(fields.marks);
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { marks() }; }

    /// @brief Create a Fields instance for #SingleNoteTremolo.
    static Fields from(unsigned marks) { return { marks }; }

    MNX_REQUIRED_PROPERTY(unsigned, marks);     ///< the number of marks (a value from 0..8, inclusive)
};

/**
 * @class Unstress
 * @brief Class that represents an unstress marking on an event
 */
class Unstress : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;
};

/**
 * @class EventMarkings
 * @brief Container for any markings on an event
 */
class EventMarkings : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_CHILD(Accent, accent);                 ///< An accent mark
    MNX_OPTIONAL_CHILD(BreathMark, breath);             ///< A breath mark
    MNX_OPTIONAL_CHILD(SoftAccent, softAccent);         ///< A soft accent mark
    MNX_OPTIONAL_CHILD(Spiccato, spiccato);             ///< A spiccato mark
    MNX_OPTIONAL_CHILD(Staccatissimo, staccatissimo);   ///< A staccatissimo mark
    MNX_OPTIONAL_CHILD(Staccato, staccato);             ///< A staccato mark
    MNX_OPTIONAL_CHILD(Stress, stress);                 ///< A stress mark
    MNX_OPTIONAL_CHILD(StrongAccent, strongAccent);     ///< A strong accent mark
    MNX_OPTIONAL_CHILD(Tenuto, tenuto);                 ///< A stress mark
    MNX_OPTIONAL_CHILD(SingleNoteTremolo, tremolo);     ///< A single-note tremolo mark
    MNX_OPTIONAL_CHILD(Unstress, unstress);             ///< A stress mark
};

} // namespace sequence
} // namespace mnx
