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
 * @namespace mnx::sequence
 * @brief classes related to sequences in the part measure
 */
namespace sequence {

/**
 * @class AccidentalEnclosure
 * @brief Represents the enclosure on an accidental.
 */
class AccidentalEnclosure : public Object
{
public:
    /// @brief Constructor for existing AccidentalEnclosure objects
    AccidentalEnclosure(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Pitch class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in the parent.
    /// @param inSymbol The enclosure symbol to use.
    AccidentalEnclosure(Base& parent, const std::string_view& key, AccidentalEnclosureSymbol inSymbol)
        : Object(parent, key)
    {
        set_symbol(inSymbol);
    }

    MNX_REQUIRED_PROPERTY(AccidentalEnclosureSymbol, symbol);      ///< The symbol to use for the enclosure
};

/**
 * @class AccidentalDisplay
 * @brief Represents an explicit directive to show or hide an accidental.
 */
class AccidentalDisplay : public Object
{
public:
    /// @brief Constructor for existing AccidentalEnclosure objects
    AccidentalDisplay(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Pitch class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in the parent.
    /// @param showAcci Show or hide the accidental.
    AccidentalDisplay(Base& parent, const std::string_view& key, bool showAcci)
        : Object(parent, key)
    {
        set_show(showAcci);
    }

    MNX_OPTIONAL_CHILD(AccidentalEnclosure, enclosure); ///< The enclosure type (brackets or parentheses). Omit if none.
    MNX_REQUIRED_PROPERTY(bool, show);                  ///< Whether to show or hide the accidental
};

/**
 * @class Rest
 * @brief Represents a rest within a musical event within a sequence.
 */
class Rest : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    MNX_OPTIONAL_PROPERTY(int, staffPosition);              ///< The staff position of non-floating rests.
};

/**
 * @class Pitch
 * @brief Represents the pitch of a note
 */
class Pitch : public Object
{
public:
    /// @brief Constructor for existing Pitch objects
    Pitch(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Pitch class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param inpStep The letter spelling of the note.
    /// @param inpOctave The octave number of the note (where C4 is middle C).
    /// @param inpAlter The chromatic alteration of the note (positive for sharp, negative for flat)
    Pitch(Base& parent, const std::string_view& key, NoteStep inpStep, int inpOctave, std::optional<int> inpAlter = std::nullopt)
        : Object(parent, key)
    {
        set_step(inpStep);
        set_octave(inpOctave);
        if (inpAlter) {
            set_alter(inpAlter.value());
        }
    }

    MNX_OPTIONAL_PROPERTY(int, alter);          ///< chromatic alteration
    MNX_REQUIRED_PROPERTY(int, octave);         ///< the octave number
    MNX_REQUIRED_PROPERTY(NoteStep, step);      ///< the note step, (i.e., "A".."G")
};

/**
 * @class Tie
 * @brief Contains information about a tie on a note.
 */
class Tie : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_PROPERTY(SlurTieEndLocation, location);    ///< mainly useful for l.v. type ties that are not connected
    MNX_OPTIONAL_PROPERTY(SlurTieSide, side);               ///< used to force tie direction (if present)
    MNX_OPTIONAL_PROPERTY(std::string, target);             ///< the octave number
};

/**
 * @class Note
 * @brief Represents a single note (i.e., within a chord) within a musical event within a sequence.
 */
class Note : public ArrayElementObject
{
public:
    /// @brief Constructor for existing Note objects
    Note(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Note class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param step The letter spelling of the note.
    /// @param octave The octave number of the note (where C4 is middle C).
    /// @param alter The chromatic alteration of the note (positive for sharp, negative for flat)
    Note(Base& parent, const std::string_view& key, NoteStep step, int octave, std::optional<int> alter = std::nullopt)
        : ArrayElementObject(parent, key)
    {
        create_pitch(step, octave, alter);
    }

    MNX_OPTIONAL_CHILD(AccidentalDisplay, accidentalDisplay);       ///< the forced show/hide state of the accidental
    MNX_OPTIONAL_NAMED_PROPERTY(std::string, styleClass, "class");  ///< style class
    MNX_OPTIONAL_PROPERTY(std::string, id);                         ///< note Id
    /// @todo `perform`
    MNX_REQUIRED_CHILD(Pitch, pitch);                               ///< the pitch of the note
    MNX_OPTIONAL_PROPERTY(std::string, smuflFont);                  ///< The SMuFL-complaint font to use for rendering the note.
    MNX_OPTIONAL_PROPERTY(int, staff);                              ///< Staff number override (e.g., for cross-staff notes.)
    MNX_OPTIONAL_CHILD(Tie, tie);                                   ///< The (forward) tie, if any.
};

/**
 * @class Event
 * @brief Represents a musical event within a sequence.
 */
class Event : public ContentObject
{
public:
    /// @brief Constructor for existing Event objects
    Event(const std::shared_ptr<json>& root, json_pointer pointer)
        : ContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Event class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param noteValue The note value
    Event(Base& parent, const std::string_view& key, std::optional<NoteValue::Initializer> noteValue = std::nullopt)
        : ContentObject(parent, key)
    {
        // per the specification, either noteValue or the full-measure boolean *must* be supplied.
        if (noteValue) {
            create_duration(noteValue.value());
        } else {
            set_measure(true);
        }
    }

    MNX_OPTIONAL_CHILD(NoteValue, duration);                ///< Symbolic duration of the event.
    MNX_OPTIONAL_PROPERTY(std::string, id);                 ///< Identifying string for the event.
    /// @todo `lyrics`
    /// @todo `markings`
    MNX_OPTIONAL_PROPERTY(bool, measure);                   ///< Whether this event is a whole-measure event.
    MNX_OPTIONAL_CHILD(Array<Note>, notes);                 ///< Note array
    /// @todo `orient`
    MNX_OPTIONAL_CHILD(Rest, rest);                         ///< indicates this event is a rest.
    /// @todo `slurs` array
    MNX_OPTIONAL_PROPERTY(int, staff);                      ///< Staff number override (e.g., for cross-staff notes.)
    MNX_OPTIONAL_PROPERTY(StemDirection, stemDirection);    ///< Forced stem direction.

    static constexpr std::string_view ContentTypeValue = "event"; ///< type value that identifies the type within the content array
};

/**
 * @class Space
 * @brief Occupies metric space without showing anything.
 */
class Space : public ContentObject
{
public:
    /// @brief Constructor for existing Space objects
    Space(const std::shared_ptr<json>& root, json_pointer pointer)
        : ContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Space class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param count the quantity of note values.
    /// @param noteValue The note value.
    Space(Base& parent, const std::string_view& key, unsigned count, const NoteValue::Initializer& noteValue)
        : ContentObject(parent, key)
    {
        create_duration(count, noteValue);
    }

    MNX_REQUIRED_CHILD(NoteValueQuantity, duration);                ///< Symbolic duration of space to occupy.

    static constexpr std::string_view ContentTypeValue = "space";   ///< type value that identifies the type within the content array
};

/**
 * @class Grace
 * @brief Represents a grace note sequence within a sequence.
 */
class Grace : public ContentObject
{
public:
    /// @brief Constructor for existing Grace objects
    Grace(const std::shared_ptr<json>& root, json_pointer pointer)
        : ContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Grace class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Grace(Base& parent, const std::string_view& key)
        : ContentObject(parent, key)
    {
        create_content();
    }

    MNX_OPTIONAL_NAMED_PROPERTY(std::string, styleClass, "class");  ///< style class
    MNX_OPTIONAL_PROPERTY(std::string, color);                      ///< color to use when rendering the grace note sequence
    MNX_REQUIRED_CHILD(ContentArray, content);                      ///< array of events
    /// @todo `graceType`
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, slash, true);          ///< whether to show a slash on the grace note

    static constexpr std::string_view ContentTypeValue = "grace";   ///< type value that identifies the type within the content array
};

/**
 * @class Tuplet
 * @brief Represents a tuplet sequence within a sequence.
 */
class Tuplet : public ContentObject
{
public:
    /// @brief Constructor for existing Tuplet objects
    Tuplet(const std::shared_ptr<json>& root, json_pointer pointer)
        : ContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Tuplet class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param innerCount The inner count: **3** quarters in the time of 2 quarters
    /// @param innerNoteValue The inner amount: 3 **quarters** in the time of 2 quarters
    /// @param outerCount The outer count: 3 quarters in the time of **2** quarters
    /// @param outerNoteValue The outer amount: 3 quarters in the time of 2 **quarters**
    Tuplet(Base& parent, const std::string_view& key,
        unsigned innerCount, const NoteValue::Initializer& innerNoteValue,
        unsigned outerCount, const NoteValue::Initializer& outerNoteValue)
        : ContentObject(parent, key)
    {
        create_inner(innerCount, innerNoteValue);
        create_outer(outerCount, outerNoteValue);
        create_content();
    }

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(AutoYesNo, bracket, AutoYesNo::Auto); ///< bracket style
    MNX_REQUIRED_CHILD(ContentArray, content);                      ///< array of events (and tuplets, at least for now)
    MNX_REQUIRED_CHILD(NoteValueQuantity, inner);                   ///< Inner quantity: **3 quarters in the time** of 2 quarters
    MNX_REQUIRED_CHILD(NoteValueQuantity, outer);                   ///< Outer quantity: 3 quarters in the time **of 2 quarters**
    /// @todo `orient`
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(TupletDisplaySetting, showNumber, TupletDisplaySetting::Inner); ///< How and whether to show the tuplet number(s)
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(TupletDisplaySetting, showValue, TupletDisplaySetting::NoNumber); ///< How and whether to show the tuplet note value(s)
    MNX_OPTIONAL_PROPERTY(int, staff);                              ///< Staff number override (e.g., for cross-staff notes.)

    static constexpr std::string_view ContentTypeValue = "tuplet";   ///< type value that identifies the type within the content array
};

} // namespace sequence

/**
 * @class Sequence
 * @brief A sequence of events and other items in this measure for a voice in a part
 */
class Sequence : public ArrayElementObject
{
public:
    /// @brief Constructor for existing Sequence objects
    Sequence(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Sequence class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Sequence(Base& parent, const std::string_view& key)
        : ArrayElementObject(parent, key)
    {
        create_content();
    }

    MNX_REQUIRED_CHILD(ContentArray, content);      ///< the content of the sequence
    /// @todo `orient` property
    MNX_OPTIONAL_PROPERTY(int, staff);              ///< the staff number for this sequence
    MNX_OPTIONAL_PROPERTY(std::string, voice);      ///< the unique (per measure) voice label for this sequence.
};

} // namespace mnx