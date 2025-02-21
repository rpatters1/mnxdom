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
#include "CommonClasses.h"

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
    /// @brief Constructor for existing Barline objects
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
    /// @brief Constructor for existing Ending objects
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
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, open, false);  ///< if this is an open (i.e., final) ending
};

/**
 * @class Fine
 * @brief Represents an Fine object (as in "D.S. al Fine")
 */
class Fine : public Object
{
public:
    /// @brief Constructor for existing Fine objects
    Fine(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Fine class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param numerator The numerator (number on top) of the fraction.
    /// @param denominator The denominator (number on bottom) of the fraction.
    Fine(Base& parent, const std::string_view& key, unsigned numerator, unsigned denominator)
        : Object(parent, key)
    {
        create_location(numerator, denominator);
    }

    MNX_OPTIONAL_NAMED_PROPERTY(std::string, styleClass, "class"); ///< style class
    MNX_OPTIONAL_PROPERTY(std::string, color);                  ///< color to use when rendering the fine direction
    MNX_REQUIRED_CHILD(RhythmicPosition, location);              ///< the location of the fine direction
};

/**
 * @class Jump
 * @brief Represents an Jump object (as in "D.S.")
 */
class Jump : public Object
{
public:
    /// @brief Constructor for existing Jump objects
    Jump(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Jump class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param jumpType The @ref JumpType of this jump.
    /// @param numerator The numerator (number on top) of the fraction.
    /// @param denominator The denominator (number on bottom) of the fraction.
    Jump(Base& parent, const std::string_view& key, JumpType jumpType, unsigned numerator, unsigned denominator)
        : Object(parent, key)
    {
        set_type(jumpType);
        create_location(numerator, denominator);
    }

    MNX_REQUIRED_PROPERTY(JumpType, type);                      ///< the JumpType
    MNX_REQUIRED_CHILD(RhythmicPosition, location);              ///< the location of the jump
};

/**
 * @class RepeatEnd
 * @brief Represents the end of a repeat in an MNX document.
 */
class RepeatEnd : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_PROPERTY(int, times);               ///< number of times to repeat
};

/**
 * @class RepeatStart
 * @brief Represents the start of a repeat in an MNX document.
 */
class RepeatStart : public Object
{
public:
    using Object::Object;

};

/**
 * @class Segno
 * @brief Represents a segno marker
 */
class Segno : public Object
{
public:
    /// @brief Constructor for existing Segno objects
    Segno(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Segno class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param numerator The numerator (number on top) of the fraction.
    /// @param denominator The denominator (number on bottom) of the fraction.
    Segno(Base& parent, const std::string_view& key, unsigned numerator, unsigned denominator)
        : Object(parent, key)
    {
        create_location(numerator, denominator);
    }

    MNX_OPTIONAL_NAMED_PROPERTY(std::string, styleClass, "class"); ///< style class
    MNX_OPTIONAL_PROPERTY(std::string, color);      ///< color to use when rendering the ending
    MNX_OPTIONAL_PROPERTY(std::string, glyph);      ///< the SMuFL glyph name to be used when rendering this segno.
    MNX_REQUIRED_CHILD(RhythmicPosition, location); ///< location
};

/**
 * @class Tempo
 * @brief Represents the tempo for a global measure.
 */
class Tempo : public ArrayElementObject
{
public:
    /// @brief Constructor for existing Tempo instances
    Tempo(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Tempo class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param bpm The number of beats per minutes
    /// @param noteValueBase The note value base for this Barline
    /// @param numDots The number of dots (may be omitted)
    Tempo(Base& parent, const std::string_view& key, int bpm, NoteValueBase noteValueBase, unsigned numDots = 0)
        : ArrayElementObject(parent, key)
    {
        set_bpm(bpm);
        create_value(noteValueBase, numDots);
    }

    MNX_REQUIRED_PROPERTY(int, bpm);                ///< the beats per minute of this tempo marking
    MNX_OPTIONAL_CHILD(RhythmicPosition, location); ///< location within the measure of the tempo marking
    MNX_REQUIRED_CHILD(NoteValue, value);           ///< the note value for the tempo.
};

/**
 * @class Measure
 * @brief Represents a single global measure instance within an MNX document.
 */
class Measure : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    MNX_OPTIONAL_CHILD(Barline, barline);           ///< optional barline for this measure
    MNX_OPTIONAL_CHILD(Ending, ending);             ///< optional ending ("volta bracket") for this measure
    MNX_OPTIONAL_CHILD(Fine, fine);                 ///< optional fine direction for this measure
    MNX_OPTIONAL_PROPERTY(int, index);              ///< the measure index which is used to refer to this measure by other classes in the MNX document
    MNX_OPTIONAL_CHILD(Jump, jump);                 ///< optional jump direction for this measure
    MNX_OPTIONAL_CHILD(KeySignature, key);          ///< optional key signature/key change for this measure
    MNX_OPTIONAL_PROPERTY(int, number);             ///< visible measure number. Use #calcMeasureIndex to get the default value.
    MNX_OPTIONAL_CHILD(RepeatEnd, repeatEnd);       ///< if present, indicates that there is backwards repeat
    MNX_OPTIONAL_CHILD(RepeatStart, repeatStart);   ///< if present, indicates that a repeated section starts here
    MNX_OPTIONAL_CHILD(Segno, segno);               ///< if present, indicates that a segno marker is here
    MNX_OPTIONAL_CHILD(Array<Tempo>, tempos);       ///< the tempo changes within the measure, if any
    MNX_OPTIONAL_CHILD(TimeSignature, time);        ///< if present, indicates a meter change

    /// @brief Calculates the barline type for this measure.
    /// @return barline().type() if barline() has a value. Otherwise the default (as defined in the MNX specification.)
    BarlineType calcBarlineType() const;

    /// @brief Calculates the meausure index for this measure.
    /// @return index() if it has a value or the default value (defined in the MNX specification) if it does not.
    int calcMeasureIndex() const;
};

/**
 * @class LyricLineMetadata
 * @brief Represents a single item of lyric metadata. The user-defined key is the lyric line ID.
 */
class LyricLineMetadata : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    MNX_OPTIONAL_PROPERTY(std::string, label);  ///< Human readable lable for this lyric line. This could be used in a software UI, for example.
    MNX_OPTIONAL_PROPERTY(std::string, lang);   ///< RFC5646 language code identifying the language of this line of lyrics.
};

/**
 * @class LyricsGlobal
 * @brief Metadata for lyrics in this MNX document
 */
class LyricsGlobal : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_CHILD(Dictionary<LyricLineMetadata>, lineMetadata); ///< Defines lyric line IDs and their metadata.
    MNX_OPTIONAL_CHILD(Array<std::string>, lineOrder);              ///< Lyric line IDs used in the document (e.g. verse numbers)
};

/**
 * @class StyleGlobal
 * @brief Visual styling selectors for this MNX document
 */
class StyleGlobal : public ArrayElementObject
{
public:
    /// @brief Constructor for existing StyleGlobal objects
    StyleGlobal(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Jump class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    /// @param selector The CSS-style selector that specifies which MNX object(s) this style applies to.
    StyleGlobal(Base& parent, const std::string_view& key, const std::string& selector)
        : ArrayElementObject(parent, key)
    {
        set_selector(selector);
    }

    MNX_OPTIONAL_PROPERTY(std::string, color);      ///< the JumpType
    MNX_REQUIRED_PROPERTY(std::string, selector);   ///< the location of the jump
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
        create_measures();
    }

    MNX_OPTIONAL_CHILD(global::LyricsGlobal, lyrics);       ///< lyrics metadata
    MNX_REQUIRED_CHILD(Array<global::Measure>, measures);   ///< array of global measures.
    MNX_OPTIONAL_CHILD(Array<global::StyleGlobal>, styles); ///< array of styles
};

} // namespace mnx