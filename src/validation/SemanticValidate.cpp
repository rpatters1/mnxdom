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
#include <map>
#include <string>

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
    void validateSequenceContent(const mnx::ContentArray& contentArray);
    void validateBeams(const mnx::Array<mnx::part::Beam>& beams, unsigned depth);
    void validateOttavas(const mnx::part::Measure& measure, const mnx::Array<mnx::part::Ottava>& ottavas);


    template <typename T, typename KeyType>
    std::optional<T> tryGetValue(const KeyType& key, const Base& location);
};

template <typename T, typename KeyType>
std::optional<T> SemanticValidator::tryGetValue(const KeyType& key, const Base& location)
{
    try {
        return document.getIdMapping().get<T>(key, location);
    } catch (const util::mapping_error&) {
        // already reported error, so fall through
    }
    return std::nullopt;
}

void SemanticValidator::validateGlobal()
{
    result.lyricLines.clear();
    if (document.global().lyrics().has_value()) {
        const auto lyricsGlobal = document.global().lyrics().value();
        const auto lineOrder = lyricsGlobal.lineOrder();
        const auto lineMetadata = lyricsGlobal.lineMetadata();
        // If both are present, validate that they match
        if (lineOrder) {
            size_t x = 0;
            for (const auto lineId : lineOrder.value()) {
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
            size_t x = 0;
            for (const auto [lineId, instance] : lineMetadata.value()) {
                if (auto rslt = result.lyricLines.emplace(lineId, instance.pointer()); !rslt.second) {
                    addError("ID \"" + lineId + "\" already exists at " + rslt.first->second.to_string(), instance);
                }
                x++;
            }
        }
    }
}

void SemanticValidator::validateSequenceContent(const mnx::ContentArray& contentArray)
{
    for (const auto content : contentArray) {
        if (content.type() == mnx::sequence::Event::ContentTypeValue) {
            auto event = content.get<mnx::sequence::Event>();
            if (event.measure()) {
                if (event.duration().has_value()) {
                    addError("Event \"" + event.id().value_or("<no-id>") + "\" has both full measure indicator and duration.", event);
                }
            } else {
                if (!event.duration().has_value()) {
                    addError("Event \"" + event.id().value_or("<no-id>") + "\" has neither full measure indicator nor duration.", event);
                }
            }
            if (event.rest().has_value()) {
                if (event.notes() && !event.notes().value().empty()) {
                    addError("Event \"" + event.id().value_or("<no-id>") + "\" is a rest but also has notes.", event);
                }
            } else {
                if (!event.notes() || event.notes().value().empty()) {
                    addError("Event \"" + event.id().value_or("<no-id>") + "\" is neither a rest nor has notes.", event);
                }
            }
            if (const auto notes = event.notes()) {
                for (const auto note : notes.value()) {
                    if (const auto ties = note.ties()) {
                        for (const auto tie : ties.value()) {
                            if (auto target = tie.target()) {
                                if (tie.lv()) {
                                    addError("Tie has both a target and is an lv tie.", tie);
                                }
                                if (const auto targetNote = tryGetValue<mnx::sequence::Note>(target.value(), tie)) {
                                    if (targetNote.value().getEnclosingElement<Part>().value().calcArrayIndex() != note.getEnclosingElement<Part>().value().calcArrayIndex()) {
                                        addError("Tie points to a note in a different part.", tie);
                                    }
                                    if (!note.pitch().isSamePitch(targetNote.value().pitch())) {
                                        addError("Tie points to a note with a different pitch.", tie);
                                    }
                                }
                            } else {
                                if (!tie.lv()) {
                                    addError("Tie has neither a target not is it an lv tie.", tie);
                                }
                            }
                        }
                    }
                }
            }
            if (!result.lyricLines.empty()) { // only check lyric lines if the line ids were provided in global.lyrics()
                if (auto lyrics = event.lyrics()) {
                    if (auto lines = lyrics.value().lines()) {
                        for (const auto line : lines.value()) {
                            if (result.lyricLines.find(line.first) == result.lyricLines.end()) {
                                addError("ID \"" + line.first + "\" not found in ID mapping", line.second);
                            }
                        }
                    }
                }
            }
            if (const auto slurs = event.slurs()) {
                for (const auto slur : slurs.value()) {
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
        }
        else if (content.type() == mnx::sequence::Grace::ContentTypeValue) {
            auto grace = content.get<mnx::sequence::Grace>();
            validateSequenceContent(grace.content());
        } else if (content.type() == mnx::sequence::Tuplet::ContentTypeValue) {
            auto tuplet = content.get<mnx::sequence::Tuplet>();
            validateSequenceContent(tuplet.content());
        }
    }
}

void SemanticValidator::validateBeams(const mnx::Array<mnx::part::Beam>& beams, unsigned depth)
{
    for (const auto beam : beams) {
        if (beam.events().size() <= 1) {
            addError("Beam contains only one or fewer events.", beam);
        }
        std::unordered_set<std::string> ids;
        std::optional<bool> isGraceBeam;
        std::optional<std::string> voice;
        for (const auto id : beam.events()) {
            if (ids.find(id) != ids.end()) {
                addError("Event \"" + id + "\" is duplicated in beam.", beam);
                continue;
            }
            ids.emplace(id);
            if (const auto event = tryGetValue<mnx::sequence::Event>(id, beam)) {
                if (isGraceBeam.has_value()) {
                    if (isGraceBeam.value() != event.value().isGrace()) {
                        addError("Event \"" + id + "\" attempts to beam a grace note to a non grace note.", beam);
                    }
                } else {
                    isGraceBeam = event.value().isGrace();
                }
                auto sequence = event.value().getEnclosingElement<mnx::Sequence>();
                if (sequence.has_value()) {
                    if (voice.has_value()) {
                        if(voice.value() != sequence.value().voice_or("")) {
                            addError("Event \"" + id + "\" attempts to beam events from different voices together.", beam);
                        }
                    } else {
                        voice = sequence.value().voice_or("");
                    }
                } else {
                    addError("Event \"" + id + "\" is not part of a sequence.", event.value());
                }
                if (auto noteValue = event.value().duration()) {
                    if (depth > noteValue.value().calcNumberOfFlags()) {
                        addError("Event \"" + id + "\" cannot have " + std::to_string(depth) + " beams", beam);
                    }
                }
            }
        }
        if (auto hooks = beam.hooks()) {
            for (const auto hook : hooks.value()) {
                if (ids.find(hook.event()) == ids.end()) {
                    addError("Hook event \"" + hook.event() + "\" is not part of the beam.", beam);
                    continue;
                }
                if (const auto event = tryGetValue<mnx::sequence::Event>(hook.event(), beam)) { // errors if the event is not found
                    if (auto noteValue = event.value().duration()) {
                        if (depth >= noteValue.value().calcNumberOfFlags()) {
                            addError("Hook event \"" + hook.event() + "\" cannot have a hook because it already has "
                                + std::to_string(depth) + " beams", beam);
                        }
                    }
                }
            }
        }
        if (auto inner = beam.inner()) {
            validateBeams(inner.value(), depth + 1);
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
        size_t numPartMeasures = part.measures() ? part.measures().value().size() : 0;
        size_t numGlobalMeasures = document.global().measures().size();
        if (numPartMeasures != numGlobalMeasures) {
            addError("Part" + partName + " contains a different number of measures (" + std::to_string(numPartMeasures)
                + ") than are defined globally (" + std::to_string(numGlobalMeasures) + ")", part);
        }
        if (auto measures = part.measures()) {
            // first pass: validateSequenceContent creates the eventList and the noteList
            for (const auto measure : measures.value()) {
                for (const auto sequence : measure.sequences()) {
                    /// @todo check voice uniqueness
                    validateSequenceContent(sequence.content());
                }
            }
            // second pass: validate other items that need a complete list of events and notes
            for (const auto measure : measures.value()) {
                if (auto beams = measure.beams()) {
                    validateBeams(beams.value(), 1);
                }
                if (auto ottavas = measure.ottavas()) {
                    validateOttavas(measure, ottavas.value());
                }
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
                        self(self, group.content());
                    } else if (element.type() == mnx::layout::Staff::ContentTypeValue) {
                        auto staff = element.get<mnx::layout::Staff>();
                        /// @todo validate "labelref"?
                        for (const auto source : staff.sources()) {
                            if (const auto part = tryGetValue<mnx::Part>(source.part(), source)) {
                                int staffNum = source.staff();
                                int numStaves = part.value().staves();
                                if (staffNum > numStaves || staffNum < 1) {
                                    addError("Layout \"" + layout.id() + "\" has invalid staff number ("
                                        + std::to_string(staffNum) + ") for part " + source.part(), source);
                                }
                            }
                            /// @todo validate "labelref"?
                            /// @todo validate "voice"?
                        }
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
                        }
                        isFirstSystem = false;
                        if (lastSystemMeasure && currentSystemMeasureIndex.value() <= lastSystemMeasure) {
                            std::string msg = currentSystemMeasureIndex < lastSystemMeasure
                                ? "starts before"
                                : "starts on the same measure as";
                            addError("Score \"" + score.name() + "\" contains system that " + msg + " previous system", system);
                        }
                        lastSystemMeasure = currentSystemMeasureIndex;
                        if (const auto layoutChanges = system.layoutChanges()) {
                            for (const auto layoutChange : layoutChanges.value()) {
                                tryGetValue<mnx::Layout>(layoutChange.layout(), layoutChange); // adds error if index not found.
                                tryGetValue<mnx::global::Measure>(layoutChange.location().measure(), layoutChange); // adds error if index not found.
                                /// @todo perhaps eventually flag location.position.fraction if it is too large for the measure
                            }
                        }
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
    validator.document.buildIdMapping([&](const std::string& message, const Base& location) {
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
