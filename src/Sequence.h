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
#include "Enumerations.h"
#include "EventMarkings.h"

namespace mnx {

class Sequence; // forward declaration

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
    /// @brief initializer class for #AccidentalEnclosure
    struct Fields
    {
        AccidentalEnclosureSymbol symbol{}; ///< the symbol to use for the enclosure
    };

    /// @brief Constructor for existing AccidentalEnclosure objects
    AccidentalEnclosure(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Pitch class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in the parent.
    /// @param fields The enclosure fields to use.
    AccidentalEnclosure(Base& parent, std::string_view key, const Fields& fields)
        : Object(parent, key)
    {
        set_symbol(fields.symbol);
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { symbol() }; }

    /// @brief Create a Fields instance for #AccidentalEnclosure.
    static Fields from(AccidentalEnclosureSymbol symbol) { return { symbol }; }

    MNX_REQUIRED_PROPERTY(AccidentalEnclosureSymbol, symbol);      ///< The symbol to use for the enclosure
};

/**
 * @class AccidentalDisplay
 * @brief Represents an explicit directive to show or hide an accidental.
 */
class AccidentalDisplay : public Object
{
public:
    /// @brief initializer class for #AccidentalDisplay
    struct Fields
    {
        bool show{}; ///< whether to show or hide the accidental
    };

    /// @brief Constructor for existing AccidentalEnclosure objects
    AccidentalDisplay(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Pitch class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in the parent.
    /// @param fields The accidental display fields to use.
    AccidentalDisplay(Base& parent, std::string_view key, const Fields& fields)
        : Object(parent, key)
    {
        set_show(fields.show);
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { show() }; }

    /// @brief Create a Fields instance for #AccidentalDisplay.
    static Fields from(bool show) { return { show }; }

    MNX_OPTIONAL_CHILD(AccidentalEnclosure, enclosure);     ///< The enclosure type (brackets or parentheses). Omit if none.
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, force, false); ///< Whether this accidental was set intentionally (e.g., a courtesy accidental).
    MNX_REQUIRED_PROPERTY(bool, show);                      ///< Whether to show or hide the accidental
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
 * @class TransposeWritten
 * @brief Represents the options for how a note is written when transposed
 */
class TransposeWritten : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, diatonicDelta, 0);  ///< the number of enharmonic transpositions to apply
};

/**
 * @class Pitch
 * @brief Represents the pitch of a note
 */
class Pitch : public Object
{
public:
    /// @brief initializer class for #Pitch
    struct Fields
    {
        NoteStep step{};    ///< the letter spelling of the note.
        int octave{};       ///< the octave number of the note (where C4 is middle C).
        int alter{};        ///< the chromatic alteration of the note (positive for sharp, negative for flat).
    };

    /// @brief Constructor for existing Pitch objects
    Pitch(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Pitch class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The fields to initialize the instance.
    Pitch(Base& parent, std::string_view key, const Fields& fields)
        : Object(parent, key)
    {
        set_step(fields.step);
        set_octave(fields.octave);
        set_or_clear_alter(fields.alter);
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { step(), octave(), alter() }; }

    /// @brief Create a Fields instance for #Pitch.
    static Fields from(NoteStep step, int octave, int alter = 0) { return { step, octave, alter }; }

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, alter, 0);  ///< chromatic alteration
    MNX_REQUIRED_PROPERTY(int, octave);         ///< the octave number
    MNX_REQUIRED_PROPERTY(NoteStep, step);      ///< the note step, (i.e., "A".."G")

    /// @brief Checks if the input pitch is the same as this pitch, including enharmonic equivalents
    /// @param src The value to compare with
    /// @return true if they are the same or enharmonically equivalent
    [[nodiscard]] bool isSamePitch(const Fields& src) const;

    /// @brief Calculates the transposed version of this pitch, taking into account the part transposition
    /// for the part that contains this pitch.
    [[nodiscard]] Fields calcTransposed() const;
};

/**
 * @class Slur
 * @brief Contains information about a tie on a note.
 */
class Slur : public ArrayElementObject
{
public:
    /// @brief initializer class for #Slur
    struct Fields
    {
        std::string target; ///< the target note id of the slur
    };

    /// @brief Constructor for existing Slur objects
    Slur(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Slur class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The slur fields to use.
    Slur(Base& parent, std::string_view key, const Fields& fields)
        : ArrayElementObject(parent, key)
    {
        set_target(fields.target);
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { target() }; }

    /// @brief Create a Fields instance for #Slur.
    static Fields from(const std::string& target) { return { target }; }

    MNX_OPTIONAL_PROPERTY(std::string, endNote);            ///< the specific note ID this slur ends on
    MNX_OPTIONAL_PROPERTY(LineType, lineType);              ///< the type of line for the slur
    MNX_OPTIONAL_PROPERTY(SlurTieSide, side);               ///< used to force slur direction (if present)
    MNX_OPTIONAL_PROPERTY(SlurTieSide, sideEnd);            ///< used to force slur's endpoint direction (if different than `side`)
    MNX_OPTIONAL_PROPERTY(std::string, startNote);          ///< the specific note ID this slur starts on
    MNX_REQUIRED_PROPERTY(std::string, target);             ///< the event ID this slur ends on (if present)
};

/**
 * @class Tie
 * @brief Contains information about a tie on a note.
 */
class Tie : public ArrayElementObject
{
public:
    /// @brief initializer class for #Tie
    struct Fields
    {
        std::optional<std::string> target; ///< the target note id of the tie (nullopt for l.v. tie)
    };

    /// @brief Constructor for existing Tie objects
    Tie(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Tie class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The tie fields to use.
    Tie(Base& parent, std::string_view key, const Fields& fields)
        : ArrayElementObject(parent, key)
    {
        if (fields.target) {
            set_target(fields.target.value());
        } else {
            set_lv(true);
        }
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { target() }; }

    /// @brief Create a Fields instance for #Tie.
    static Fields from(const std::optional<std::string>& target = std::nullopt) { return { target }; }

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, lv, false);    ///< Indicates the presence of an l.v. tie (instead of target).
    MNX_OPTIONAL_PROPERTY(SlurTieSide, side);               ///< Used to force tie direction (if present).
    MNX_OPTIONAL_PROPERTY(std::string, target);             ///< The note id of the tied-to note.
    MNX_OPTIONAL_PROPERTY(TieTargetType, targetType);       ///< The type of target for this tie (if present).
};

/**
 * @class NoteBase
 * @brief Represents common elements between @ref Note and @ref KitNote
 */
class NoteBase : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    /// @todo `perform`
    MNX_OPTIONAL_PROPERTY(int, staff);                              ///< Staff number override (e.g., for cross-staff notes.)
    MNX_OPTIONAL_CHILD(Array<Tie>, ties);                           ///< The (forward) ties, if any.
};

/**
 * @class KitNote
 * @brief Represents a single note in a (drum) kit within a musical event within a sequence.
 */
class KitNote : public NoteBase
{
public:
    /// @brief initializer class for #KitNote
    struct Fields
    {
        std::string kitComponentId; ///< the ID within the kit for this part
    };

    /// @brief Constructor for existing Note objects
    KitNote(const std::shared_ptr<json>& root, json_pointer pointer)
        : NoteBase(root, pointer)
    {
    }

    /// @brief Creates a new Note class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The kit note fields to use.
    KitNote(Base& parent, std::string_view key, const Fields& fields)
        : NoteBase(parent, key)
    {
        set_kitComponent(fields.kitComponentId);
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { kitComponent() }; }

    /// @brief Create a Fields instance for #KitNote.
    static Fields from(const std::string& kitComponentId) { return { kitComponentId }; }

    MNX_REQUIRED_PROPERTY(std::string, kitComponent);               ///< The ID within the kit for this part.

    inline static constexpr std::string_view JsonSchemaTypeName = "kit-note";     ///< required for mapping
};

/**
 * @class Note
 * @brief Represents a single note (i.e., within a chord) within a musical event within a sequence.
 */
class Note : public NoteBase
{
public:
    /// @brief initializer class for #Note
    struct Fields
    {
        Pitch::Fields pitch{}; ///< the pitch of the note
    };

    /// @brief Constructor for existing Note objects
    Note(const std::shared_ptr<json>& root, json_pointer pointer)
        : NoteBase(root, pointer)
    {
    }

    /// @brief Creates a new Note class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The note fields to use.
    Note(Base& parent, std::string_view key, const Fields& fields)
        : NoteBase(parent, key)
    {
        create_pitch(fields.pitch);
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { pitch() }; }

    /// @brief Create a Fields instance for #Note.
    static Fields from(const Pitch::Fields& pitch) { return { pitch }; }

    MNX_OPTIONAL_CHILD(AccidentalDisplay, accidentalDisplay);       ///< the forced show/hide state of the accidental
    MNX_REQUIRED_CHILD(Pitch, pitch);                               ///< the pitch of the note
    MNX_OPTIONAL_CHILD(TransposeWritten, written);                  ///< How to write this note when it is displayed transposed

    inline static constexpr std::string_view JsonSchemaTypeName = "note";     ///< required for mapping
};

/**
 * @class EventLyricLine
 * @brief Contains information about a lyric syllable from one lyric line on a note.
 */
class EventLyricLine : public ArrayElementObject
{
public:
    /// @brief initializer class for #EventLyricLine
    struct Fields
    {
        std::string syllableText; ///< the syllable text
    };

    /// @brief Constructor for existing EventLyricLine objects
    EventLyricLine(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new EventLyricLine class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The lyric line fields to use.
    EventLyricLine(Base& parent, std::string_view key, const Fields& fields)
        : ArrayElementObject(parent, key)
    {
        set_text(fields.syllableText);
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { text() }; }

    /// @brief Create a Fields instance for #EventLyricLine.
    static Fields from(const std::string& syllableText) { return { syllableText }; }

    MNX_REQUIRED_PROPERTY(std::string, text);           ///< the syllable text
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(LyricLineType, type, LyricLineType::Whole);  ///< the type of syllable (in relation to the complete word)
};

/**
 * @class EventLyrics
 * @brief Contains information about the lyric syllables on the event.
 */
class EventLyrics : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_CHILD(Dictionary<EventLyricLine>, lines);      ///< the syllables per lyric line
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
    Event(Base& parent, std::string_view key)
        : ContentObject(parent, key)
    {
    }

    MNX_OPTIONAL_CHILD(NoteValue, duration);                ///< Symbolic duration of the event.
    MNX_OPTIONAL_CHILD(Array<KitNote>, kitNotes);           ///< KitNote array for percussion kit notation.
    MNX_OPTIONAL_CHILD(EventLyrics, lyrics);                ///< The lyric syllables on this event.
    MNX_OPTIONAL_CHILD(EventMarkings, markings);            ///< Articulation markings on this event.
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, measure, false); ///< Whether this event is a whole-measure event.
    MNX_OPTIONAL_CHILD(Array<Note>, notes);                 ///< Note array
    /// @todo `orient`
    MNX_OPTIONAL_CHILD(Rest, rest);                         ///< indicates this event is a rest.
    MNX_OPTIONAL_CHILD(Array<Slur>, slurs);                 ///< The slurs that start on this event.
    MNX_OPTIONAL_PROPERTY(int, staff);                      ///< Staff number override (e.g., for cross-staff events.)
    MNX_OPTIONAL_PROPERTY(StemDirection, stemDirection);    ///< Forced stem direction.

    /// @brief Finds a note in the event by its ID
    /// @param noteId The note ID to find
    /// @return The note if found, otherwise std::nullopt;
    [[nodiscard]] std::optional<Note> findNote(const std::string& noteId) const;

    /// @brief Returns true if this event is part of a grace note sequence.
    [[nodiscard]] bool isGrace() const;

    /// @brief Returns true if this event is part of a multi-note tremolo sequence.
    [[nodiscard]] bool isTremolo() const;

    /// @brief Returns the @ref Sequence instance for this event
    /// @throws std::logic_error if the json pointer does not contain a sequence (should be impossible)
    [[nodiscard]] Sequence getSequence() const;

    /// @brief Returns the index of the event (or its container) in the @ref Sequence instance for this event
    /// @throws std::logic_error if the json pointer does not contain a sequence (should be impossible)
    [[nodiscard]] size_t getSequenceIndex() const;

    /// @brief Calculates and returns the duration of this event.
    [[nodiscard]]  FractionValue calcDuration() const;

    /// @brief Calculates and returns the start time of this event within the measure.
    [[nodiscard]] FractionValue calcStartTime() const;

    inline static constexpr std::string_view ContentTypeValue = ContentObject::ContentTypeValueDefault; ///< type value that identifies the type within the content array
    inline static constexpr std::string_view JsonSchemaTypeName = "event";     ///< required for mapping
};

/**
 * @class Space
 * @brief Occupies metric space without showing anything.
 */
class Space : public ContentObject
{
public:
    /// @brief initializer class for #Space
    struct Fields
    {
        FractionValue duration{}; ///< duration of the space
    };

    /// @brief Constructor for existing Space objects
    Space(const std::shared_ptr<json>& root, json_pointer pointer)
        : ContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Space class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The space fields to use.
    Space(Base& parent, std::string_view key, const Fields& fields)
        : ContentObject(parent, key)
    {
        create_duration({ fields.duration });
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { duration() }; }

    /// @brief Create a Fields instance for #Space.
    static Fields from(const FractionValue& duration) { return { duration }; }

    MNX_REQUIRED_CHILD(Fraction, duration);               ///< Duration of space to occupy.

    inline static constexpr std::string_view ContentTypeValue = "space";   ///< type value that identifies the type within the content array
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
    Grace(Base& parent, std::string_view key)
        : ContentObject(parent, key)
    {
        create_content();
    }

    MNX_OPTIONAL_PROPERTY(std::string, color);                      ///< color to use when rendering the grace note sequence
    MNX_REQUIRED_CHILD(ContentArray, content);                      ///< array of events
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(GraceType, graceType, GraceType::StealPrevious); ///< The playback type of the grace note.
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, slash, true);          ///< whether to show a slash on the grace note

    inline static constexpr std::string_view ContentTypeValue = "grace";   ///< type value that identifies the type within the content array
};

/**
 * @class MultiNoteTremolo
 * @brief Represents a multi-note tremolo sequence within a sequence.
 */
class MultiNoteTremolo : public ContentObject
{
public:
    /// @brief initializer class for #MultiNoteTremolo
    struct Fields
    {
        int numberOfMarks{};                   ///< the number of marks (beams)
        NoteValueQuantity::Fields noteValueQuant{}; ///< the note value quantity
    };

    /// @brief Constructor for existing Tuplet objects
    MultiNoteTremolo(const std::shared_ptr<json>& root, json_pointer pointer)
        : ContentObject(root, pointer)
    {
    }

    /// @brief Creates a new MultiNoteTremolo class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The tremolo fields to use.
    MultiNoteTremolo(Base& parent, std::string_view key, const Fields& fields)
        : ContentObject(parent, key)
    {
        create_content();
        set_marks(fields.numberOfMarks);
        create_outer(fields.noteValueQuant);
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { marks(), outer() }; }

    /// @brief Create a Fields instance for #MultiNoteTremolo.
    static Fields from(int numberOfMarks, const NoteValueQuantity::Fields& noteValueQuant)
    { return { numberOfMarks, noteValueQuant }; }

    MNX_REQUIRED_CHILD(ContentArray, content);                      ///< array of events
    MNX_OPTIONAL_CHILD(NoteValue, individualDuration);              ///< optional value that specifies the individual duration of each event in the tremolo.
    MNX_REQUIRED_PROPERTY(int, marks);                              ///< the number of marks (beams)
    MNX_REQUIRED_CHILD(NoteValueQuantity, outer);                   ///< a half note tremolo would be 2 quarters here

    inline static constexpr std::string_view ContentTypeValue = "tremolo";   ///< type value that identifies the type within the content array
};

/**
 * @class Tuplet
 * @brief Represents a tuplet sequence within a sequence.
 */
class Tuplet : public ContentObject
{
public:
    /// @brief initializer class for #Tuplet
    struct Fields
    {
        NoteValueQuantity::Fields innerNoteValueQuant{}; ///< inner amount
        NoteValueQuantity::Fields outerNoteValueQuant{}; ///< outer amount
    };

    /// @brief Constructor for existing Tuplet objects
    Tuplet(const std::shared_ptr<json>& root, json_pointer pointer)
        : ContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Tuplet class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fields The tuplet fields to use.
    Tuplet(Base& parent, std::string_view key, const Fields& fields)
        : ContentObject(parent, key)
    {
        create_inner(fields.innerNoteValueQuant);
        create_outer(fields.outerNoteValueQuant);
        create_content();
    }

    /// @brief Implicit conversion back to Fields.
    operator Fields() const { return { inner(), outer() }; }

    /// @brief Create a Fields instance for #Tuplet.
    static Fields from(const NoteValueQuantity::Fields& innerNoteValueQuant,
                       const NoteValueQuantity::Fields& outerNoteValueQuant)
    { return { innerNoteValueQuant, outerNoteValueQuant }; }

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(AutoYesNo, bracket, AutoYesNo::Auto); ///< bracket style
    MNX_REQUIRED_CHILD(ContentArray, content);                      ///< array of events, tuplets, and grace notes
    MNX_REQUIRED_CHILD(NoteValueQuantity, inner);                   ///< Inner quantity: **3 quarters in the time** of 2 quarters
    MNX_REQUIRED_CHILD(NoteValueQuantity, outer);                   ///< Outer quantity: 3 quarters in the time **of 2 quarters**
    /// @todo `orient`
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(TupletDisplaySetting, showNumber, TupletDisplaySetting::Inner); ///< How and whether to show the tuplet number(s)
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(TupletDisplaySetting, showValue, TupletDisplaySetting::NoNumber); ///< How and whether to show the tuplet note value(s)
    MNX_OPTIONAL_PROPERTY(int, staff);                              ///< Staff number override (e.g., for cross-staff notes.)

    /// @brief Return the triplet ratio as a FractionValue
    [[nodiscard]] FractionValue ratio() const
    { return outer() / inner(); }

    inline static constexpr std::string_view ContentTypeValue = "tuplet";   ///< type value that identifies the type within the content array
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
    Sequence(Base& parent, std::string_view key)
        : ArrayElementObject(parent, key)
    {
        create_content();
    }

    MNX_REQUIRED_CHILD(ContentArray, content);          ///< the content of the sequence
    /// @todo `orient` property
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staff, 1);  ///< the staff number for this sequence
    MNX_OPTIONAL_PROPERTY(std::string, voice);          ///< the unique (per measure) voice label for this sequence.
};

} // namespace mnx
