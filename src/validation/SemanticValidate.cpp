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

private:
    void validateMeasure(const mnx::part::Measure& measure);
    void validateSequenceContent(const mnx::ContentArray& contentArray);

    template <typename KeyType, typename ElementType>
    bool addKey(const KeyType& key, std::unordered_map<KeyType, ElementType>& keyMap, const ElementType& value, const Base& instance);

    template <typename KeyType, typename ElementType>
    std::optional<ElementType> getValue(const KeyType& key, const std::unordered_map<KeyType, ElementType>& keyMap, const Base& instance);
};

template <typename KeyType, typename ElementType>
bool SemanticValidator::addKey(const KeyType& key, std::unordered_map<KeyType, ElementType>& keyMap, const ElementType& value, const Base& instance)
{
    auto it = keyMap.find(key);
    if (it == keyMap.end()) {
        keyMap.emplace(key, value);
    } else {
        std::string keyString = [key]() {
            if constexpr (std::is_same_v<KeyType, std::string>) {
                return "\"" + key + "\"";
            } else {
                return std::to_string(key);
            }
        }();
        if constexpr (std::is_same_v<mnx::json_pointer, ElementType>) {
            result.errors.emplace_back(instance.pointer(), instance.ref(), "ID " + keyString + " already exists at " + it->second.to_string());
        } else {
            result.errors.emplace_back(instance.pointer(), instance.ref(), "ID " + keyString + " already exists at index " + std::to_string(it->second));
        }
        return false;
    }
    return true;
}

template <typename KeyType, typename ElementType>
std::optional<ElementType> SemanticValidator::getValue(const KeyType& key, const std::unordered_map<KeyType, ElementType>& keyMap, const Base& instance)
{
    auto it = keyMap.find(key);
    if (it != keyMap.end()) {
        return it->second;
    }
    std::string keyString = [key]() {
        if constexpr (std::is_same_v<KeyType, std::string>) {
            return "\"" + key + "\"";
        } else {
            return std::to_string(key);
        }
    }();
    result.errors.emplace_back(instance.pointer(), instance.ref(), "ID " + keyString + " not found in key value list.");
    return std::nullopt;
}

void SemanticValidator::validateGlobal()
{
    int measureId = 0;
    result.measureCount = 0;
    result.measureList.clear();
    for (const auto meas : document.global().measures()) {
        result.measureCount++;
        measureId = meas.index_or(measureId + 1);
        addKey(measureId, result.measureList, meas.calcArrayIndex(), meas);
    }
    result.lyricLines.clear();
    if (document.global().lyrics().has_value()) {
        const auto lyricsGlobal = document.global().lyrics().value();
        const auto lineOrder = lyricsGlobal.lineOrder();
        const auto lineMetadata = lyricsGlobal.lineMetadata();
        // If both are present, validate that they match
        if (lineOrder) {
            size_t x = 0;
            for (const auto lineId : lineOrder.value()) {
                addKey(lineId, result.lyricLines, x, lineOrder.value()[x]);
                x++;
            }
            if (lineMetadata) {
                if (result.lyricLines.size() != lineMetadata.value().size()) {
                    result.errors.emplace_back(lineMetadata.value().pointer(), lineMetadata.value().ref(), "Size of line metadata does not match size of line order.");
                }
                for (const auto& [lineId, instance] : *lineMetadata) {
                    getValue(lineId, result.lyricLines, instance); // reports error if not found
                }
            }
        } else if (lineMetadata) {
            size_t x = 0;
            for (const auto [lineId, instance] : lineMetadata.value()) {
                addKey(lineId, result.lyricLines, x, instance);
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
            if (event.id().has_value()) {
                addKey(event.id().value(), result.eventList, event.pointer(), event);
            }
            if (event.measure()) {
                if (event.duration().has_value()) {
                    result.errors.emplace_back(event.pointer(), event.ref(), "Event \"" + event.id().value_or("<no-id>") + "\" has both full measure indicator and duration.");
                }
            } else {
                if (!event.duration().has_value()) {
                    result.errors.emplace_back(event.pointer(), event.ref(), "Event \"" + event.id().value_or("<no-id>") + "\" has neither full measure indicator nor duration.");
                }
            }
            if (event.rest().has_value()) {
                if (event.notes() && !event.notes().value().empty()) {
                    result.errors.emplace_back(event.pointer(), event.ref(), "Event \"" + event.id().value_or("<no-id>") + "\" is a rest but also has notes.");
                }
            } else {
                if (!event.notes() || event.notes().value().empty()) {
                    result.errors.emplace_back(event.pointer(), event.ref(), "Event \"" + event.id().value_or("<no-id>") + "\" is neither a rest nor has notes.");
                }
            }
            if (const auto notes = event.notes()) {
                for (const auto note : notes.value()) {
                    if (note.id().has_value()) {
                        addKey(note.id().value(), result.noteList, note.pointer(), note);
                    }
                    if (note.ties()) {
                        result.notesWithTies.emplace(note.pointer());
                    }
                }
            }
            if (!result.lyricLines.empty()) { // only check lyric lines if the line ids were provided in global.lyrics()
                if (auto lyrics = event.lyrics()) {
                    if (auto lines = lyrics.value().lines()) {
                        for (const auto line : lines.value()) {
                            getValue(line.first, result.lyricLines, line.second); // validates the line id.
                        }
                    }
                }
            }
        } else if (content.type() == mnx::sequence::Grace::ContentTypeValue) {
            auto grace = content.get<mnx::sequence::Grace>();
            validateSequenceContent(grace.content());
        } else if (content.type() == mnx::sequence::Tuplet::ContentTypeValue) {
            auto tuplet = content.get<mnx::sequence::Tuplet>();
            validateSequenceContent(tuplet.content());
        }
    }
}

void SemanticValidator::validateMeasure(const mnx::part::Measure& measure)
{
    for (const auto sequence : measure.sequences()) {
        /// @todo check voice uniqueness
        validateSequenceContent(sequence.content());
    }
}

void SemanticValidator::validateParts()
{
    result.partList.clear();
    result.eventList.clear();
    result.noteList.clear();
    for (const auto part : document.parts()) {
        size_t x = part.calcArrayIndex();
        std::string partName = "[" + std::to_string(x) + "]";
        if (auto partId = part.id()) {
            addKey(partId.value(), result.partList, x, part);
            partName = " \"" + partId.value() + "\"";
        }
        size_t numMeasures = part.measures() ? part.measures().value().size() : 0;
        if (numMeasures != result.measureCount) {
            result.errors.emplace_back(part.pointer(), part.ref(), "Part" + partName + " contains a different number of measures ("
                + std::to_string(numMeasures) + ") than are defined globally (" + std::to_string(result.measureCount) + ")");
        }
        if (auto measures = part.measures(); measures) {
            for (const auto measure : measures.value()) {
                validateMeasure(measure);
            }
        }
    }
    for (const auto& ptr : result.notesWithTies) {
        mnx::sequence::Note tiedNote(document.root(), json_pointer(ptr));
        MNX_ASSERT_IF(!tiedNote.ties()) {
            throw std::logic_error("The notesWithTies array contains a note with no ties.");
        }
        auto ties = tiedNote.ties().value();
        for (const auto tie : ties) {
            if (auto target = tie.target()) {
                if (auto targetNotePtr = getValue(target.value(), result.noteList, tie)) {
                    mnx::sequence::Note targetNote(document.root(), targetNotePtr.value());
                    if (targetNote.getPart().value().calcArrayIndex() != tiedNote.getPart().value().calcArrayIndex()) {
                        result.errors.emplace_back(tie.pointer(), tie.ref(), "Tie points to a note in a different part.");
                    }
                    if (!tiedNote.pitch().isSamePitch(targetNote.pitch())) {
                        result.errors.emplace_back(tie.pointer(), tie.ref(), "Tie points to a note with a different pitch.");
                    }
                }
            }
        }
    }
}

void SemanticValidator::validateLayouts()
{
    result.layoutList.clear();
    if (auto layouts = document.layouts()) {  // layouts are *not* required in MNX
        for (const auto layout : layouts.value()) {
            addKey(layout.id(), result.layoutList, layout.calcArrayIndex(), layout);
            auto validateContent = [&](auto&& self, const mnx::ContentArray& content) -> void {
                for (const auto element : content) {
                    if (element.type() == mnx::layout::Group::ContentTypeValue) {
                        auto group = element.get<mnx::layout::Group>();
                        self(self, group.content());
                    } else if (element.type() == mnx::layout::Staff::ContentTypeValue) {
                        auto staff = element.get<mnx::layout::Staff>();
                        /// @todo validate "labelref"?
                        for (const auto source : staff.sources()) {
                            if (auto index = getValue(source.part(), result.partList, source)) {
                                int staffNum = source.staff();
                                const auto part = document.parts()[index.value()];
                                int numStaves = part.staves();
                                if (staffNum > numStaves || staffNum < 1) {
                                    result.errors.emplace_back(source.pointer(), source.ref(), "Layout \"" + layout.id() + "\" has invalid staff number ("
                                        + std::to_string(staffNum) + ") for part " + source.part());
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
                getValue(layout.value(), result.layoutList, score); // adds error if index not found.
            }
            if (const auto multimeasureRests = score.multimeasureRests()) {
                for (const auto mmRest : multimeasureRests.value()) {
                    if (auto index = getValue(mmRest.start(), result.measureList, mmRest)) {
                        if (index.value() + mmRest.duration() > result.measureCount) {
                            result.errors.emplace_back(mmRest.pointer(), mmRest.ref(), "Multimeasure rest at measure "
                                + std::to_string(mmRest.start()) + " in score \""
                                + score.name() + "\" spans non-existent measures");
                        }
                    }
                }
            }
            std::optional<size_t> lastSystemMeasure;
            bool isFirstSystem = true;
            if (auto pages = score.pages()) {
                for (const auto page : pages.value()) {
                    if (const auto layout = page.layout()) {
                        getValue(layout.value(), result.layoutList, page); // adds error if index not found.
                    }
                    for (const auto system : page.systems()) {
                        if (const auto layout = system.layout()) {
                            getValue(layout.value(), result.layoutList, system); // adds error if index not found.
                        }
                        auto currentSystemMeasure = getValue(system.measure(), result.measureList, system);
                        if (currentSystemMeasure) {
                            if (isFirstSystem && currentSystemMeasure.value() > 0) {
                                result.errors.emplace_back(system.pointer(), system.ref(), "The first system in score \"" + score.name()
                                    + "\" starts after the first measure");
                            }
                        }
                        isFirstSystem = false;
                        if (lastSystemMeasure && currentSystemMeasure <= lastSystemMeasure) {
                            std::string msg = currentSystemMeasure < lastSystemMeasure
                                ? "starts before"
                                : "starts on the same measure as";
                            result.errors.emplace_back(system.pointer(), system.ref(), "Score \"" + score.name() +
                                "\" contains system that " + msg + " previous system");
                        }
                        lastSystemMeasure = currentSystemMeasure;
                        if (const auto layoutChanges = system.layoutChanges()) {
                            for (const auto layoutChange : layoutChanges.value()) {
                                getValue(layoutChange.layout(), result.layoutList, layoutChange); // adds error if index not found.
                                getValue(layoutChange.location().measure(), result.measureList, layoutChange); // adds error if index not found.
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

    // these calls are order-dependent
    validator.validateGlobal();
    validator.validateParts();
    validator.validateLayouts();
    validator.validateScores();

    return validator.result;
}

} // namespace validation
} // namespace mnx
