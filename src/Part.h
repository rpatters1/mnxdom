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
    }

    /// @brief Creates a new Beam class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.¥
    Beam(Base& parent, std::string_view key)
        : ArrayElementObject(parent, key)
    {
        create_events();
    }

    MNX_OPTIONAL_CHILD(Array<Beam>, beams);                 ///< the beams that comprise the next beam level (may be omitted)
    MNX_OPTIONAL_PROPERTY(BeamHookDirection, direction);    ///< the forced direction of a beam hook (if this beam contains one event).
    MNX_REQUIRED_CHILD(Array<std::string>, events);         ///< the events that comprise this beam level

    inline static constexpr std::string_view JsonSchemaTypeName = "beam";     ///< required for mapping
};

/**
 * @class Clef
 * @brief Represents a visible clef in the measure
 */
class Clef : public Object
{
public:
    /// @brief initializer class for #Clef
    struct Fields
    {
        ClefSign clefSign{};                                     ///< the type of clef symbol
        int staffPosition{};                                     ///< staff position offset from center of staff
        std::optional<OttavaAmountOrZero> octaveAdjustment;      ///< optional octave adjustment
    };

    /// @brief Constructor for existing Clef instances
    Clef(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Clef class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The clef fields to use.
    Clef(Base& parent, std::string_view key, const Fields& fields)
        : Object(parent, key)
    {
        set_sign(fields.clefSign);
        set_staffPosition(fields.staffPosition);
        if (fields.octaveAdjustment.has_value()) {
            set_octave(fields.octaveAdjustment.value());
        }
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { sign(), staffPosition(), octave() }; }

    /// @brief Create a Fields instance for #Clef.
    static Fields from(ClefSign clefSign, int staffPosition,
                       std::optional<OttavaAmountOrZero> octaveAdjustment = std::nullopt)
    { return { clefSign, staffPosition, octaveAdjustment }; }

    MNX_OPTIONAL_PROPERTY(std::string, color);      ///< color to use when rendering the ending
    MNX_OPTIONAL_PROPERTY(std::string, glyph);      ///< the specific SMuFL glyph to use for rendering the clef
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(OttavaAmountOrZero, octave, OttavaAmountOrZero::NoTransposition);  ///< the number of octaves by which the clef transposes
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, showOctave, true); ///< if octave is non-zero, this value determines whether the octave should be displayed on the clef
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
    /// @brief initializer class for #Dynamic
    struct Fields
    {
        std::string value;          ///< the value of the dynamic
        FractionValue position{};   ///< the position within the measure
    };

    /// @brief Constructor for existing Space objects
    Dynamic(const std::shared_ptr<json>& root, json_pointer pointer)
        : ContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Space class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The dynamic fields to use.
    Dynamic(Base& parent, std::string_view key, const Fields& fields)
        : ContentObject(parent, key)
    {
        set_value(fields.value);
        create_position({ fields.position });
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { value(), position().fraction() }; }

    /// @brief Create a Fields instance for #Dynamic.
    static Fields from(const std::string& value, const FractionValue& position) { return { value, position }; }

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
class       Ottava : public ArrayElementObject
{
public:
    /// @brief initializer class for #Ottava
    struct Fields
    {
        OttavaAmount value{};        ///< the value (type) of ottava
        FractionValue position{};    ///< the start position of the ottava
        int endMeasureId{};          ///< the end measure of the ottava
        FractionValue endPosition{}; ///< the position within the end measure
    };

    /// @brief Constructor for existing Space objects
    Ottava(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Space class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The ottava fields to use.
    Ottava(Base& parent, std::string_view key, const Fields& fields)
        : ArrayElementObject(parent, key)
    {
        create_position({ fields.position });
        create_end({ fields.endMeasureId, fields.endPosition });
        set_value(fields.value);
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const
    {
        return { value(), position().fraction(), end().measure(), end().position().fraction() };
    }

    /// @brief Create a Fields instance for #Ottava.
    static Fields from(OttavaAmount value, const FractionValue& position, int endMeasureId,
                       const FractionValue& endPosition)
    { return { value, position, endMeasureId, endPosition }; }

    MNX_REQUIRED_CHILD(MeasureRhythmicPosition, end);               ///< The end of the ottava (includes any events starting at this location)
    /// @todo orient
    MNX_REQUIRED_CHILD(RhythmicPosition, position);                 ///< The start position of the ottava
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staff, 1);              ///< The staff (within the part) this ottava applies to
    MNX_REQUIRED_PROPERTY(OttavaAmount, value);                     ///< The type of ottava (amount of displacement, in octaves)
    MNX_OPTIONAL_PROPERTY(std::string, voice);                      ///< Optionally specify the voice this ottava applies to.
};

/**
 * @class KitComponent
 * @brief Describes a single instrument of a drum kit associated with the part.
 */
class KitComponent : public ArrayElementObject
{
public:
    /// @brief initializer class for #KitComponent
    struct Fields
    {
        int staffPosition{}; ///< the staff position of the kit component
    };

    /// @brief Constructor for existing Clef instances
    KitComponent(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Clef class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The kit component fields to use.
    KitComponent(Base& parent, std::string_view key, const Fields& fields)
        : ArrayElementObject(parent, key)
    {
        set_staffPosition(fields.staffPosition);
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { staffPosition() }; }

    /// @brief Create a Fields instance for #KitComponent.
    static Fields from(int staffPosition) { return { staffPosition }; }

    MNX_OPTIONAL_PROPERTY(std::string, name);       ///< Human-readable name of the kit component
    MNX_OPTIONAL_PROPERTY(std::string, sound);      ///< The sound ID in `global.sounds`.
    MNX_REQUIRED_PROPERTY(int, staffPosition);      ///< The staff position of the kit component, where 0 is the middle line.
};

/**
 * @class PartTransposition
 * @brief Describes a part's instrument transposition
 */
class PartTransposition : public Object
{
public:
    /// @brief initializer class for #PartTransposition
    struct Fields
    {
        Interval::Fields interval{}; ///< the transposition interval for the part
    };

    /// @brief Constructor for existing Clef instances
    PartTransposition(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Clef class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The transposition fields to use.
    PartTransposition(Base& parent, std::string_view key, const Fields& fields)
        : Object(parent, key)
    {
        create_interval(fields.interval);
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { interval() }; }

    /// @brief Create a Fields instance for #PartTransposition.
    static Fields from(const Interval::Fields& interval) { return { interval }; }

    MNX_REQUIRED_CHILD(Interval, interval);         ///< the transposition interval
    MNX_OPTIONAL_PROPERTY(int, keyFifthsFlipAt);    ///< the number of sharps (positive) or flats (negative) at which to simplify the key signature
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, prefersWrittenPitches, false); ///< if true, this instrument prefers displaying written pitches even in the context of
                                                    ///< concert-pitch score. (Examples could be piccolo, double base, glockenspiel, etc.)

    /// @brief Calculates and returns the transposed keyFifths value for the input key.
    /// @param concertKey The concert key to calculate from.
    KeySignature::Fields calcTransposedKey(const KeySignature::Fields& concertKey) const;
};

/**
 * @class PositionedClef
 * @brief Represents a positioned clef for the measure
 */
class PositionedClef : public ArrayElementObject
{
public:
    /// @brief initializer class for #PositionedClef
    struct Fields
    {
        ClefSign clefSign{};                                     ///< the type of clef symbol
        int staffPosition{};                                     ///< staff position offset from center of staff
        std::optional<OttavaAmountOrZero> octaveAdjustment;      ///< optional octave adjustment
    };

    /// @brief Constructor for existing PositionedClef instances
    PositionedClef(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new PositionedClef class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The positioned clef fields to use.
    PositionedClef(Base& parent, std::string_view key, const Fields& fields)
        : ArrayElementObject(parent, key)
    {
        create_clef({ fields.clefSign, fields.staffPosition, fields.octaveAdjustment });
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { clef().sign(), clef().staffPosition(), clef().octave() }; }

    /// @brief Create a Fields instance for #PositionedClef.
    static Fields from(ClefSign clefSign, int staffPosition,
                       std::optional<OttavaAmountOrZero> octaveAdjustment = std::nullopt)
    { return { clefSign, staffPosition, octaveAdjustment }; }

    MNX_REQUIRED_CHILD(Clef, clef);                     ///< the beats per minute of this tempo marking
    MNX_OPTIONAL_CHILD(RhythmicPosition, position);     ///< location within the measure of the tempo marking
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staff, 1);  ///< the staff number (for multistaff parts)
};

/**
 * @class Measure
 * @brief Represents a single measure in a part in an MNX document. It contains the majority of the musical information in its sequences.
 */
class Measure : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    /// @brief Creates a new Measure class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Measure(Base& parent, std::string_view key)
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

    /// @brief Returns the global measure for this part measure.
    /// @throws std::logic_error if the global measure does not exist.
    [[nodiscard]] mnx::global::Measure getGlobalMeasure() const;

    /// @brief Caculates the time signature at this measure.
    /// @return The time signagure or std::nullopt if none.
    [[nodiscard]] std::optional<TimeSignature> calcCurrentTime() const;

    inline static constexpr std::string_view JsonSchemaTypeName = "part-measure";     ///< required for mapping
};

} // namespace part

/**
 * @class Part
 * @brief Represents a single part in an MNX document.
 */
class Part : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    /// @brief Creates a new Part class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Part(Base& parent, std::string_view key)
        : ArrayElementObject(parent, key)
    {
        // required children
        create_measures();
    }

    MNX_OPTIONAL_CHILD(Dictionary<part::KitComponent>, kit); ///< The definition of a kit of (usually percussion) instruments that are used by the part.
    MNX_REQUIRED_CHILD(Array<part::Measure>, measures); ///< Contains all the musical data for this part
    MNX_OPTIONAL_PROPERTY(std::string, name);           ///< Specifies the user-facing full name of this part
    MNX_OPTIONAL_PROPERTY(std::string, shortName);      ///< Specifies the user-facing abbreviated name of this part
    MNX_OPTIONAL_PROPERTY(std::string, smuflFont);      ///< Name of SMuFL-font for notation elements in the part (can be overridden by children)
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staves, 1); ///< The number of staves in this part.
    MNX_OPTIONAL_CHILD(part::PartTransposition, transposition); ///< the instrument transposition for the part

    inline static constexpr std::string_view JsonSchemaTypeName = "measure-global";     ///< required for mapping
};

} // namespace mnx
