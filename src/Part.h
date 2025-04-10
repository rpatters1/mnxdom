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
#include "Sequence.h"

namespace mnx {

/**
 * @namespace mnx::part
 * @brief classes related to the root parts array
 */
namespace part {

/**
 * @class BeamHook
 * @brief Represents a hooked beam. (A beam on a single note, broken on the hightest beam numbers.)
 */
class BeamHook : public ArrayElementObject
{
public:
    using Initializer = std::pair<std::string, BeamHookDirection>;  ///< used to initialize a BeamHook instance.
                                                                    ///< The values are the event ID and the hook direction.

    /// @brief Constructor for existing BeamHook objects
    BeamHook(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new BeamHook class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param hookInfo The event ID and direction of the hook.
    BeamHook(Base& parent, const std::string_view& key, const Initializer& hookInfo)
        : ArrayElementObject(parent, key)
    {
        set_event(hookInfo.first);
        set_direction(hookInfo.second);
    }

    MNX_REQUIRED_PROPERTY(BeamHookDirection, direction);        ///< the direction of the hook.
    MNX_REQUIRED_PROPERTY(std::string, event);                  ///< The event with the hook.
};

/**
 * @class Beam
 * @brief Contains information about each level of bea
 */
class Beam : public ArrayElementObject
{
public:
    /// @brief Constructor for existing Beam objects
    Beam(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
        //create_events();
    }

    /// @brief Creates a new Beam class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.¥
    Beam(Base& parent, const std::string_view& key)
        : ArrayElementObject(parent, key)
    {
        create_events();
    }

    MNX_REQUIRED_CHILD(Array<std::string>, events);     ///< the events that comprise this beam level
    MNX_OPTIONAL_CHILD(Array<BeamHook>, hooks);         ///< the beam hooks at this level
    MNX_OPTIONAL_CHILD(Array<Beam>, inner);             ///< the beams that comprise the next beam level
};

/**
 * @class Clef
 * @brief Represents a visible clef in the measure
 */
class Clef : public Object
{
public:
    /// @brief Constructor for existing Clef instances
    Clef(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Clef class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param staffPosition The note value base for this Barline
    /// @param clefSign The type of clef symbold
    /// @param octaveAdjustment The number of octaves by which the clef transposes (may be omitted)
    Clef(Base& parent, const std::string_view& key, int staffPosition, ClefSign clefSign, std::optional<int> octaveAdjustment = std::nullopt)
        : Object(parent, key)
    {
        set_staffPosition(staffPosition);
        set_sign(clefSign);
        if (octaveAdjustment.has_value()) {
            set_octave(octaveAdjustment.value());
        }
    }

    MNX_OPTIONAL_NAMED_PROPERTY(std::string, styleClass, "class"); ///< style class
    MNX_OPTIONAL_PROPERTY(std::string, color);      ///< color to use when rendering the ending
    MNX_OPTIONAL_PROPERTY(std::string, glyph);      ///< the specific SMuFL glyph to use for rendering the clef
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, octave, 0);  ///< the number of octaves by which the clef transposes
    MNX_REQUIRED_PROPERTY(ClefSign, sign);          ///< the clef sign
    MNX_REQUIRED_PROPERTY(int, staffPosition);      ///< staff position offset from center of staff (in half-spaces)
};

/**
 * @class Dynamic
 * @brief Represents a dynamic positioned with the next event in the sequence.
 */
class Dynamic : public ContentObject
{
public:
    /// @brief Constructor for existing Space objects
    Dynamic(const std::shared_ptr<json>& root, json_pointer pointer)
        : ContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Space class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param value the value of the dynamic. Currently the spec allows any string here.
    /// @param position the position of the dynamic within the measure.
    Dynamic(Base& parent, const std::string_view& key, const std::string& value, const Fraction::Initializer& position)
        : ContentObject(parent, key)
    {
        set_value(value);
        create_position(position);
    }

    MNX_OPTIONAL_PROPERTY(std::string, glyph);                      ///< The SMuFL glyph name (if any)
    MNX_REQUIRED_CHILD(RhythmicPosition, position);                 ///< The rhythmic position of the dynamic within the measure.
    MNX_OPTIONAL_PROPERTY(int, staff);                              ///< The staff (within the part) this dynamic applies to
    MNX_REQUIRED_PROPERTY(std::string, value);                      ///< The value of the dynamic. Currently the MNX spec allows any string here.
    MNX_OPTIONAL_PROPERTY(std::string, voice);                      ///< Optionally specify the voice this dynamic applies to.
};

/**
 * @class Ottava
 * @brief Represents an ottava starting with the next event in the sequence
 */
class Ottava : public ArrayElementObject
{
public:
    /// @brief Constructor for existing Space objects
    Ottava(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Space class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param value the value (type) of ottava.
    /// @param position the start position of the ottava.
    /// @param endMeasureId The end measure of the ottava.
    /// @param endPosition The position within the end measure of the ottava. (The ottava includes events that start at this position.)
    Ottava(Base& parent, const std::string_view& key, OttavaAmount value, const Fraction::Initializer& position, int endMeasureId, const Fraction::Initializer& endPosition)
        : ArrayElementObject(parent, key)
    {
        create_position(position);
        create_end(endMeasureId, endPosition);
        set_value(value);
    }

    MNX_REQUIRED_CHILD(MeasureRhythmicPosition, end);               ///< The end of the ottava (includes any events starting at this location)
    /// @todo orient
    MNX_REQUIRED_CHILD(RhythmicPosition, position);                 ///< The start position of the ottava
    MNX_OPTIONAL_PROPERTY(int, staff);                              ///< The staff (within the part) this ottava applies to
    MNX_REQUIRED_PROPERTY(OttavaAmount, value);                     ///< The type of ottava (amount of displacement, in octaves)
    MNX_OPTIONAL_PROPERTY(std::string, voice);                      ///< Optionally specify the voice this ottava applies to.
};

/**
 * @class PositionedClef
 * @brief Represents a positioned clef for the measure
 */
class PositionedClef : public ArrayElementObject
{
public:
    /// @brief Constructor for existing PositionedClef instances
    PositionedClef(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new PositionedClef class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param staffPosition The note value base for this Barline
    /// @param clefSign The type of clef symbold
    /// @param octaveAdjustment The number of octaves by which the clef transposes (may be omitted)
    PositionedClef(Base& parent, const std::string_view& key, int staffPosition, ClefSign clefSign, std::optional<int> octaveAdjustment = std::nullopt)
        : ArrayElementObject(parent, key)
    {
        create_clef(staffPosition, clefSign, octaveAdjustment);
    }

    MNX_REQUIRED_CHILD(Clef, clef);                     ///< the beats per minute of this tempo marking
    MNX_OPTIONAL_CHILD(RhythmicPosition, position);     ///< location within the measure of the tempo marking
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staff, 1);  ///< the staff number (for multistaff parts)
};

/**
 * @class Measure
 * @brief Represents a single measuer in a part in an MNX document. It contains the majority of the musical information in its sequences.
 */
class Measure : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    /// @brief Creates a new Measure class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Measure(Base& parent, const std::string_view& key)
        : ArrayElementObject(parent, key)
    {
        // required children
        create_sequences();
    }

    MNX_OPTIONAL_CHILD(Array<Beam>, beams);             ///< the beams in this measure
    MNX_OPTIONAL_CHILD(Array<PositionedClef>, clefs);   ///< the clef changes in this bar
    MNX_OPTIONAL_CHILD(Array<Dynamic>, dynamics);       ///< the dynamics in this measure
    MNX_OPTIONAL_CHILD(Array<Ottava>, ottavas);         ///< the ottavas in this measure
    MNX_REQUIRED_CHILD(Array<Sequence>, sequences);     ///< sequences that contain all the musical details in each measure
};

} // namespace Part

/**
 * @class Part
 * @brief Represents a single part in an MNX document.
 */
class Part : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    MNX_OPTIONAL_PROPERTY(std::string, id);             ///< Uniquely identifies the part
    MNX_OPTIONAL_CHILD(Array<part::Measure>, measures); ///< Contains all the musical data for this part
    MNX_OPTIONAL_PROPERTY(std::string, name);           ///< Specifies the user-facing full name of this part
    MNX_OPTIONAL_PROPERTY(std::string, shortName);      ///< Specifies the user-facing abbreviated name of this part
    MNX_OPTIONAL_PROPERTY(std::string, smuflFont);      ///< Name of SMuFL-font for notation elements in the part (can be overridden by children)
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staves, 1); ///< The number of staves in this part.
};

} // namespace mnx