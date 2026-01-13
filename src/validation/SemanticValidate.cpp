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
#include <memory>
#include <unordered_map>
#include <set>
#include <map>
#include <string>
#include <vector>

#include "mnxdom.h"

namespace mnx {
namespace validation {

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

class SemanticValidator
{
public:
    SemanticValidator(const Document& doc) : document(doc) {}

    SemanticValidationResult result;
    Document document;

    void validateGlobal();
    void validateParts();
    void validateLayouts();
    void validateScores();

    void addError(const std::string& message, const Base& location)
    {
        result.errors.emplace_back(location.pointer(), location.ref(), message);
    }

private:
    void validateSequenceContent(const mnx::ContentArray& contentArray, const Base& location,
        FractionValue expectedDuration, bool allowEventsOnly = false, bool requireExactDuration = false, FractionValue* actualElapsedOut = nullptr);
    void validateBeams(const mnx::Array<mnx::part::Beam>& beams, unsigned depth);
    void validateOttavas(const mnx::part::Measure& measure, const mnx::Array<mnx::part::Ottava>& ottavas);

    template <typename NoteType>
    void validateTies(const mnx::Array<mnx::sequence::Tie>& ties, const NoteType& note);

    template <typename T, typename KeyType>
    std::optional<T> tryGetValue(const KeyType& key, const Base& errorLocation);
};

template <typename T, typename KeyType>
std::optional<T> SemanticValidator::tryGetValue(const KeyType& key, const Base& errorLocation)
{
    try {
        return document.getEntityMap().get<T>(key, errorLocation);
    } catch (const util::mapping_error&) {
        // already reported error, so fall through
    }
    return std::nullopt;
}

void SemanticValidator::validateGlobal()
{
    auto global = document.global();

    result.lyricLines.clear();
    if (global.lyrics().has_value()) {
        const auto lyricsGlobal = global.lyrics().value();
        const auto lineOrder = lyricsGlobal.lineOrder();
        const auto lineMetadata = lyricsGlobal.lineMetadata();
        // If both are present, validate that they match
        if (lineOrder) {
            size_t x = 0;
            for (const auto& lineId : lineOrder.value()) {
                if (auto rslt = result.lyricLines.emplace(lineId, lineOrder.value()[x].pointer()); !rslt.second) {
                    addError("ID \"" + lineId + "\" already exists at " + rslt.first->second.to_string(), lineOrder.value()[x]);
                }
                x++;
            }
            if (lineMetadata) {
                if (result.lyricLines.size() != lineMetadata.value().size()) {
                    addError("Size of line metadata does not match size of line order.", lineMetadata.value());
                }
                for (const auto& [lineId, instance] : lineMetadata.value()) {
                    if (result.lyricLines.find(lineId) == result.lyricLines.end()) {
                        addError("ID \"" + lineId + "\" not found in ID mapping", instance);
                    }
                }
            }
        } else if (lineMetadata) {
            for (const auto& [lineId, instance] : lineMetadata.value()) {
                if (auto rslt = result.lyricLines.emplace(lineId, instance.pointer()); !rslt.second) {
                    addError("ID \"" + lineId + "\" already exists at " + rslt.first->second.to_string(), instance);
                }
            }
        }
    }

    if (const auto sounds = global.sounds()) {
        for (const auto& [soundId, sound] : sounds.value()) {
            if (auto midiNumber = sound.midiNumber()) {
                if (midiNumber.value() < 0 || midiNumber.value() > 127) {
                    addError("Invalid midi number: " + std::to_string(midiNumber.value()), sound);
                }
            }
        }
    }
}

template <typename NoteType>
void SemanticValidator::validateTies(const mnx::Array<mnx::sequence::Tie>& ties, const NoteType& note)
{
    static_assert(std::is_base_of_v<mnx::sequence::NoteBase, NoteType>, "NoteType must be derived from NoteBase.");

    std::optional<mnx::Part> enclosingPart = note.template getEnclosingElement<Part>();
    if (!enclosingPart) {
        addError("The tied note has no part.", note);
        return;
    }

    for (const auto& tie : ties) {
        if (auto target = tie.target()) {
            if (tie.lv()) {
                addError("Tie has both a target and is an lv tie.", tie);
            }
            if (const auto targetNote = tryGetValue<NoteType>(target.value(), tie)) {
                if (targetNote->template getEnclosingElement<Part>()->calcArrayIndex() != enclosingPart->calcArrayIndex()) {
                    addError("Tie points to a note in a different part.", tie);
                }
                if constexpr (std::is_same_v<NoteType, mnx::sequence::Note>) {
                    if (!note.pitch().isSamePitch(targetNote->pitch())) {
                        addError("Tie points to a note with a different pitch.", tie);
                    }
                } else if constexpr (std::is_same_v<NoteType, mnx::sequence::KitNote>) {
                    if (targetNote->kitComponent() != note.kitComponent()) {
                        addError("Tie points to a different kit component.", tie);
                    }
                }
            } else {
                addError("Tie points to non-existent target " + target.value(), tie);
            }
        } else {
            if (!tie.lv()) {
                addError("Tie has neither a target nor is it an lv tie.", tie);
            } else if (tie.targetType()) {
                addError("Tie is an lv tie but also has targetType.", tie);
            }
        }
    }
}

void SemanticValidator::validateSequenceContent(const mnx::ContentArray& contentArray, const Base& location,
    FractionValue expectedDuration, bool allowEventsOnly, bool requireExactDuration, FractionValue* actualElapsedOut)
{
    if (actualElapsedOut) {
        *actualElapsedOut = 0;
    }
    auto part = contentArray.getEnclosingElement<mnx::Part>();
    if (!part.has_value()) {
        addError("Sequence content array has no part.", contentArray);
        return;
    }
    auto elapsedTime = FractionValue(0);
    for (const auto content : contentArray) {
        if (content.type() == mnx::sequence::Event::ContentTypeValue) {
            auto event = content.get<mnx::sequence::Event>();
            if (event.measure()) {
                if (event.duration().has_value()) {
                    addError("Event \"" + event.id_or("<no-id>") + "\" has both full measure indicator and duration.", event);
                }
            } else {
                if (!event.duration().has_value()) {
                    addError("Event \"" + event.id_or("<no-id>") + "\" has neither full measure indicator nor duration.", event);
                }
            }
            elapsedTime += event.calcDuration();
            if (event.rest().has_value()) {
                if (event.notes() && !event.notes().value().empty()) {
                    addError("Event \"" + event.id_or("<no-id>") + "\" is a rest but also has notes.", event);
                }
            } else {
                const bool notesExist = event.notes() && !event.notes().value().empty();
                const bool kitNotesExist = event.kitNotes() && !event.kitNotes().value().empty();
                if (!notesExist && !kitNotesExist) {
                    addError("Event \"" + event.id_or("<no-id>") + "\" is neither a rest nor has notes.", event);
                }
            }
            if (const auto notes = event.notes()) {
                for (const auto note : notes.value()) {
                    const int noteAlter = note.pitch().alter();
                    if (std::abs(noteAlter) > 3) {
                        addError("Note \"" + note.id_or("<no-id>") + "\" has alteration value " + std::to_string(noteAlter) + ". MNX files are limited to +/-3.", note);
                    }
                    if (const auto ties = note.ties()) {
                        validateTies(ties.value(), note);
                    }
                }
            }
            if (const auto kitNotes = event.kitNotes()) {
                for (const auto kitNote : kitNotes.value()) {
                    if (!part->kit() || !part->kit()->contains(kitNote.kitComponent())) {
                        addError("Kit note has kit element " + kitNote.kitComponent() + " that is not defined in the part's kit.", kitNote);
                    }
                    if (const auto ties = kitNote.ties()) {
                        validateTies(ties.value(), kitNote);
                    }
                }
            }
            if (!result.lyricLines.empty()) { // only check lyric lines if the line ids were provided in global.lyrics()
                if (auto lyrics = event.lyrics()) {
                    if (auto lines = lyrics.value().lines()) {
                        for (const auto& line : lines.value()) {
                            if (result.lyricLines.find(line.first) == result.lyricLines.end()) {
                                addError("ID \"" + line.first + "\" not found in ID mapping", line.second);
                            }
                        }
                    }
                }
            }
            if (const auto slurs = event.slurs()) {
                for (const auto& slur : slurs.value()) {
                    const auto targetEvent = tryGetValue<mnx::sequence::Event>(slur.target(), slur);
                    if (slur.endNote()) {
                        bool foundNote = false;
                        if (targetEvent) {
                            foundNote = targetEvent.value().findNote(slur.endNote().value()).has_value();
                        }
                        if (!foundNote) {
                            addError("Slur contains end note \"" + slur.endNote().value() + "\" that does not exist in target.", slur);
                        }
                    }
                    if (slur.startNote()) {
                        if (!event.findNote(slur.startNote().value())) {
                            addError("Slur contains start note \"" + slur.endNote().value()
                                + "\" that does not exist in the containing event.", slur);
                        }
                    }
                }
            }
        } else if (content.type() == mnx::sequence::Grace::ContentTypeValue) {
            if (allowEventsOnly) {
                addError("Content array contains grace note object, which is not permitted for type " + content.type(), content);
            }
            auto grace = content.get<mnx::sequence::Grace>();
            validateSequenceContent(grace.content(), grace, 0, /*allowEventsOnly*/true); // true => error on content other than events
        } else if (content.type() == mnx::sequence::Tuplet::ContentTypeValue) {
            if (allowEventsOnly) {
                addError("Content array contains tuplet object, which is not permitted for type " + content.type(), content);
            }
            auto tuplet = content.get<mnx::sequence::Tuplet>();
            FractionValue tupletElapsed;
            validateSequenceContent(tuplet.content(), tuplet, tuplet.inner(), /*allowEventsOnly*/false, /*requireExactDuration*/true, &tupletElapsed);
            if (tuplet.inner() != 0) {
                elapsedTime += tupletElapsed * tuplet.ratio();
            } else {
                addError("Encountered tuplet with zero length inner value.", tuplet);
            }
        } else if (content.type() == mnx::sequence::MultiNoteTremolo::ContentTypeValue) {
            if (allowEventsOnly) {
                addError("Content array contains multi-note tremolo object, which is not permitted for type " + content.type(), content);
            }
            auto tremolo = content.get<mnx::sequence::MultiNoteTremolo>();
            const auto multiple = tremolo.outer().multiple();
            if (multiple < 2) {
                addError("Multi-note tremolo has " + std::to_string(tremolo.outer().multiple()) + " events.", tremolo);
            }
            if (multiple > 0) {
                FractionValue tremoloElapsed;
                const FractionValue expectedInner = tremolo.outer() * multiple;
                validateSequenceContent(tremolo.content(), tremolo, expectedInner, /*allowEventsOnly*/true, /*requireExactDuration*/true, &tremoloElapsed);
            }
            elapsedTime += tremolo.outer();
        } else if (content.type() == mnx::sequence::Space::ContentTypeValue) {
            if (allowEventsOnly) {
                addError("Content array contains space object, which is not permitted for type " + content.type(), content);
            }
            auto space = content.get<mnx::sequence::Space>();
            elapsedTime += space.duration();
        } else {
            addError("Unknown content type \"" + content.type() + "\" encounterd in layout.", content);
        }
    }
    if (expectedDuration != 0) {
        if (elapsedTime > expectedDuration) {
            addError("Entries in content array add up to more than the expected value.", location);
        } else if (elapsedTime < expectedDuration && requireExactDuration) {
            addError("Entries in content array add up to less than the expected value.", location);
        }
    }
    if (actualElapsedOut) {
        *actualElapsedOut = elapsedTime;
    }
}

void SemanticValidator::validateBeams(const mnx::Array<mnx::part::Beam>& beams, unsigned depth)
{
    std::set<std::pair<unsigned, std::string>> ids;
    for (const auto beam : beams) {
        if (beam.events().empty()) {
            addError("Beam contains no events.", beam);
            continue;
        }
        const auto beamMeasure = beam.getEnclosingElement<mnx::part::Measure>();
        if (!beamMeasure) {
            addError("Unable to find enclosing measure for beam.", beam);
            continue;
        }
        size_t currentMeasureIndex = beamMeasure.value().calcArrayIndex();
        bool requireMeasuresEqualOnFirst = depth == 1;
        std::optional<bool> isGraceBeam;
        std::optional<std::string> voice;
        FractionValue currentSequenceTime = 0;
        for (const auto id : beam.events()) {
            if (ids.find(std::make_pair(depth, id)) != ids.end()) {
                addError("Event \"" + id + "\" is duplicated in beam at depth " + std::to_string(depth) + ".", beam);
                continue;
            }
            ids.emplace(std::make_pair(depth, id));
            if (const auto event = tryGetValue<mnx::sequence::Event>(id, beam)) {
                size_t nextMeasureIndex = currentMeasureIndex;
                if (const auto eventMeasure = event.value().getEnclosingElement<mnx::part::Measure>()) {
                    nextMeasureIndex = eventMeasure.value().calcArrayIndex();
                } else {
                    addError("Unable to find enclosing measure for event.", event.value());
                }
                const auto startTime = event.value().calcStartTime();
                if (requireMeasuresEqualOnFirst && nextMeasureIndex != currentMeasureIndex) {
                    addError("First event in beam is not in the same measure as the beam.", beam);
                } else if (nextMeasureIndex < currentMeasureIndex) {
                    addError("Beam measures are out of sequence", beam);
                } else if (nextMeasureIndex > currentMeasureIndex) {
                    currentSequenceTime = startTime;
                }
                requireMeasuresEqualOnFirst = false;
                currentMeasureIndex = nextMeasureIndex;
                const auto nextSequenceTime = startTime;
                if (nextSequenceTime < currentSequenceTime) {
                    addError("Beam events are out of sequence.", beam);
                }
                currentSequenceTime = nextSequenceTime;
                if (event.value().isTremolo()) {
                    addError("Beam containing event \"" + id + "\" is actually a multi-note tremolo and should not be a beam.", beam);
                    continue;
                }
                if (isGraceBeam.has_value()) {
                    if (isGraceBeam.value() != event.value().isGrace()) {
                        addError("Event \"" + id + "\" attempts to beam a grace note to a non grace note.", beam);
                    }
                } else {
                    isGraceBeam = event.value().isGrace();
                }
                if (voice.has_value()) {
                    if(voice.value() != event.value().getSequence().voice_or("")) {
                        addError("Event \"" + id + "\" attempts to beam events from different voices together.", beam);
                    }
                } else {
                    voice = event.value().getSequence().voice_or("");
                }
                if (auto noteValue = event.value().duration()) {
                    if (depth > noteValue.value().calcNumberOfFlags()) {
                        addError("Event \"" + id + "\" cannot have " + std::to_string(depth) + " beams", beam);
                    }
                }
            }
        }
        if (beam.direction().has_value() && beam.events().size() != 1) {
            addError("Beam specifies a hook direction but contains more than one event.", beam);
        }
        if (auto beams = beam.beams()) {
            validateBeams(beams.value(), depth + 1);
        }
    }
}

void SemanticValidator::validateOttavas(const mnx::part::Measure& measure, const mnx::Array<mnx::part::Ottava>& ottavas)
{    
    for (const auto ottava : ottavas) {
        if (auto endMeasure = tryGetValue<mnx::global::Measure>(ottava.end().measure(), ottava)) {
            size_t thisMeasureIndex = measure.calcArrayIndex();
            size_t endMeasureInbdex = endMeasure.value().calcArrayIndex();
            if (thisMeasureIndex > endMeasureInbdex) {
                addError("Ottava ends before it begins", ottava);
            } else if (thisMeasureIndex == endMeasureInbdex && ottava.position().fraction() > ottava.end().position().fraction()) {
                addError("Ottava ends before it begins (in the same measure)", ottava);
            }
        }
    }
}

void SemanticValidator::validateParts()
{
    for (const auto part : document.parts()) {
        size_t x = part.calcArrayIndex();
        std::string partName = "[" + std::to_string(x) + "]";
        const auto measures = part.measures();
        size_t numPartMeasures = measures.size();
        size_t numGlobalMeasures = document.global().measures().size();
        if (numPartMeasures != numGlobalMeasures) {
            addError("Part" + partName + " contains a different number of measures (" + std::to_string(numPartMeasures)
                + ") than are defined globally (" + std::to_string(numGlobalMeasures) + ")", part);
            if (numGlobalMeasures < numPartMeasures) {
                return; // cannot continue because can't get current time for part measures greater than global measures
            }
        }
        if (part.staves() < 1) {
            addError("Part" + partName + " contains no staves (" + std::to_string(part.staves()) + ")", part);
        }
        if (auto kit = part.kit()) {
            auto sounds = document.global().sounds();
            for (const auto& [kitElementId, kitElement] : kit.value()) {
                if (kitElement.sound()) {
                    if (!sounds || !sounds->contains(kitElement.sound().value())) {
                        addError("Sound ID " + kitElement.sound().value() + " is not defined in global.sounds.", kitElement);
                    }
                }
            }
        }
        const int staffCount = part.staves();
        if (numPartMeasures > 0) {
            const auto firstMeasure = measures[0];
            std::vector<bool> staffHasInitialClef(static_cast<size_t>(staffCount) + 1, false);
            if (auto clefs = firstMeasure.clefs()) {
                for (const auto clef : clefs.value()) {
                    const int staffNumber = clef.staff();
                    if (staffNumber < 1 || staffNumber > staffCount) {
                        continue;
                    }
                    bool atMeasureStart = true;
                    if (const auto position = clef.position()) {
                        const FractionValue offset = position->fraction();
                        if (offset != FractionValue(0, 1)) {
                            atMeasureStart = false;
                        }
                    }
                    if (atMeasureStart) {
                        staffHasInitialClef[static_cast<size_t>(staffNumber)] = true;
                    }
                }
            }
            for (int staffNumber = 1; staffNumber <= staffCount; ++staffNumber) {
                /// @todo eventually we should not skip part.kit, but mnx currently has no percussion clef.
                if (!part.kit() && !staffHasInitialClef[static_cast<size_t>(staffNumber)]) {
                    addError("Missing clef at the beginning of staff " + std::to_string(staffNumber) +
                                 " in part " + part.id_or("<no-id>") + " (first measure).",
                             firstMeasure);
                }
            }
        }
        // first pass: validateSequenceContent creates the eventList and the noteList
        for (const auto measure : measures) {
            if (auto clefs = measure.clefs()) {
                for (const auto clef : clefs.value()) {
                    const int staffNumber = clef.staff();
                    if (staffNumber < 1 || staffNumber > staffCount) {
                        addError("Clef references non-existent staff " + std::to_string(staffNumber) +
                                     " in part " + part.id_or("<no-id>") + ".",
                                 clef);
                    }
                }
            }
            auto measureTime = [&]() -> FractionValue {
                if (auto time = measure.calcCurrentTime()) {
                    return time.value();
                }
                return FractionValue(4, 4);
            }();
            for (const auto sequence : measure.sequences()) {
                if (sequence.staff() > part.staves()) {
                    addError("Sequence references non-existent part staff for part " + part.id_or("<no-id>") + ".", sequence);
                    continue;
                }
                /// @todo check voice uniqueness
                validateSequenceContent(sequence.content(), sequence, measureTime);
            }
        }
        // second pass: validate other items that need a complete list of events and notes
        for (const auto measure : measures) {
            if (auto beams = measure.beams()) {
                validateBeams(beams.value(), 1);
            }
            if (auto ottavas = measure.ottavas()) {
                validateOttavas(measure, ottavas.value());
            }
        }
    }
}

void SemanticValidator::validateLayouts()
{
    if (auto layouts = document.layouts()) {  // layouts are *not* required in MNX
        for (const auto layout : layouts.value()) {
            auto validateContent = [&](auto&& self, const mnx::ContentArray& content) -> void {
                for (const auto element : content) {
                    if (element.type() == mnx::layout::Group::ContentTypeValue) {
                        auto group = element.get<mnx::layout::Group>();
                        if (group.content().empty()) {
                            addError("Layout group contains no content.", group);
                        }
                        self(self, group.content());
                    } else if (element.type() == mnx::layout::Staff::ContentTypeValue) {
                        auto staff = element.get<mnx::layout::Staff>();
                        if (!util::analyzeLayoutStaffVoices(staff)) {
                            addError("Layout staff \"" + staff.id_or("<no-id>") + "\" has one or more part voices specified multiple times.", staff);
                        }
                        /// @todo validate "labelref"?
                        for (const auto source : staff.sources()) {
                            if (const auto part = tryGetValue<mnx::Part>(source.part(), source)) {
                                int staffNum = source.staff();
                                int numStaves = part.value().staves();
                                if (staffNum > numStaves || staffNum < 1) {
                                    addError("Layout \"" + layout.id_or("<no-id>") + "\" has invalid staff number ("
                                        + std::to_string(staffNum) + ") for part " + source.part(), source);
                                }
                            }
                            /// @todo validate "labelref"?
                            /// @todo validate "voice"?
                        }
                    } else {
                        addError("Unknown content type \"" + element.type() + "\" encounterd in layout.", element);
                    }
                }
            };
            validateContent(validateContent, layout.content());
        }
    }
}

void SemanticValidator::validateScores()
{
    if (const auto scores = document.scores()) {  // scores are *not* required in MNX
        for (const auto score : scores.value()) {
            if (const auto layout = score.layout()) {
                tryGetValue<mnx::Layout>(layout.value(), score); // adds error if index not found.
            }
            if (const auto multimeasureRests = score.multimeasureRests()) {
                for (const auto mmRest : multimeasureRests.value()) {
                    if (const auto measure = tryGetValue<mnx::global::Measure>(mmRest.start(), mmRest)) {
                        size_t index = measure.value().calcArrayIndex();
                        if (index + mmRest.duration() > document.global().measures().size()) {
                            addError("Multimeasure rest at measure " + std::to_string(mmRest.start()) + " in score \""
                                + score.name() + "\" spans non-existent measures", mmRest);
                        }
                    }
                }
            }
            std::optional<size_t> lastSystemMeasure;
            bool isFirstSystem = true;
            bool skipScore = false;
            if (auto pages = score.pages()) {
                for (const auto page : pages.value()) {
                    if (const auto layout = page.layout()) {
                        tryGetValue<mnx::Layout>(layout.value(), page); // adds error if index not found.
                    }
                    for (const auto system : page.systems()) {
                        if (const auto layout = system.layout()) {
                            tryGetValue<mnx::Layout>(layout.value(), system); // adds error if index not found.
                        }
                        std::optional<size_t> currentSystemMeasureIndex;
                        if (const auto currentSystemMeasure = tryGetValue<mnx::global::Measure>(system.measure(), system)) {
                            currentSystemMeasureIndex = currentSystemMeasure.value().calcArrayIndex();
                            if (isFirstSystem && currentSystemMeasureIndex.value() > 0) {
                                addError("The first system in score \"" + score.name()
                                    + "\" starts after the first measure", system);
                            }
                        } else {
                            addError("Score \"" + score.name()
                                + "\" references missing measure " + std::to_string(system.measure()), system);
                            skipScore = true;
                            break;
                        }
                        isFirstSystem = false;
                        if (lastSystemMeasure && currentSystemMeasureIndex && currentSystemMeasureIndex.value() <= lastSystemMeasure) {
                            std::string msg = currentSystemMeasureIndex.value() < lastSystemMeasure.value()
                                ? "starts before"
                                : "starts on the same measure as";
                            addError("Score \"" + score.name() + "\" contains system that " + msg + " previous system", system);
                        }
                        lastSystemMeasure = currentSystemMeasureIndex;
                        if (const auto layoutChanges = system.layoutChanges()) {
                            for (const auto layoutChange : layoutChanges.value()) {
                                tryGetValue<mnx::Layout>(layoutChange.layout(), layoutChange); // adds error if index not found.
                                if (const auto globalMeasure = tryGetValue<mnx::global::Measure>(layoutChange.location().measure(), layoutChange)) { // adds error if index not found.
                                    if (layoutChange.location().position().fraction() >= globalMeasure->calcCurrentTime()) {
                                        addError("Layout \"" + layoutChange.id_or("<no-id>") + "\" starts at or past the end of the measure.", layoutChange);
                                    }
                                }
                            }
                        }
                    }
                    if (skipScore) {
                        break;
                    }
                }
            }
        }
    }
}

#endif // DOXYGEN_SHOULD_IGNORE_THIS

SemanticValidationResult semanticValidate(const Document& document)
{
    SemanticValidator validator(document);
    validator.document.buildEntityMap({}, [&](const std::string& message, const Base& location) {
        validator.addError(message, location);
    });

    // these calls are order-dependent
    validator.validateGlobal();
    validator.validateParts();
    validator.validateLayouts();
    validator.validateScores();

    return validator.result;
}

} // namespace validation
} // namespace mnx
