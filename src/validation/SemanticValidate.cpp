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
    template <typename KeyType>
    bool addKey(const KeyType& key, std::unordered_map<KeyType, size_t>& keyMap, size_t index, const Base& instance);

    template <typename KeyType>
    std::optional<size_t> getIndex(const KeyType& key, const std::unordered_map<KeyType, size_t>& keyMap, const Base& instance);
};

template <typename KeyType>
bool SemanticValidator::addKey(const KeyType& key, std::unordered_map<KeyType, size_t>& keyMap, size_t index, const Base& instance)
{
    auto it = keyMap.find(key);
    if (it == keyMap.end()) {
        keyMap.emplace(key, index);
    } else {
        std::string keyString = [key]() {
            if constexpr (std::is_same_v<KeyType, std::string>) {
                return "\"" + key + "\"";
            } else {
                return std::to_string(key);
            }   
        }();
        result.errors.emplace_back(instance.pointer(), instance.ref(), "ID " + keyString + " already exists at index " + std::to_string(it->second));
        return false;
    }
    return true;
}

template <typename KeyType>
std::optional<size_t> SemanticValidator::getIndex(const KeyType& key, const std::unordered_map<KeyType, size_t>& keyMap, const Base& instance)
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
    result.errors.emplace_back(instance.pointer(), instance.ref(), "ID " + keyString + " not found in key index list.");
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
}

void SemanticValidator::validateParts()
{
    result.partList.clear();
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
        /// @todo sequences
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
                            if (auto index = getIndex(source.part(), result.partList, source)) {
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
                getIndex(layout.value(), result.layoutList, score); // adds error if index not found.
            }
            if (const auto multimeasureRests = score.multimeasureRests()) {
                for (const auto mmRest : multimeasureRests.value()) {
                    if (auto index = getIndex(mmRest.start(), result.measureList, mmRest)) {
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
                        getIndex(layout.value(), result.layoutList, page); // adds error if index not found.
                    }
                    for (const auto system : page.systems()) {
                        if (const auto layout = system.layout()) {
                            getIndex(layout.value(), result.layoutList, system); // adds error if index not found.
                        }
                        auto currentSystemMeasure = getIndex(system.measure(), result.measureList, system);
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
                                getIndex(layoutChange.layout(), result.layoutList, layoutChange); // adds error if index not found.
                                getIndex(layoutChange.location().measure(), result.measureList, layoutChange); // adds error if index not found.
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
