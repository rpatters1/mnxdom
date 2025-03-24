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

class SemanticValidator
{
public:
    SemanticValidator(const Document& doc) : document(doc) {}
    
    ValidationResult result;
    Document document;
    std::map<int, size_t> measureList; // key: measId; value: index of measure in global measure array
    size_t measureCount{}; // can be different than measureList.size() if there is a duplicate key error
    std::unordered_map<std::string, size_t> partList;
    std::unordered_map<std::string, size_t> layoutList;

    bool validGlobal{};

    void validateGlobal();
};

void SemanticValidator::validateGlobal()
{
    validGlobal = true;
    int measureId = 0;
    measureCount = 0;
    for (const auto meas : document.global().measures()) {
        measureCount++;
        measureId = meas.index_or(measureId + 1);
        auto it = measureList.find(measureId);
        if (it == measureList.end()) {
            measureList.emplace(measureId, meas.calcArrayIndex());
        } else {
            //logMessage(LogMsg() << "measure index " + std::to_string(measureId) + " is duplicated at location "
            //    + std::to_string(it->second) + " and " + std::to_string(meas.calcArrayIndex()) + ".", LogSeverity::Error);
            validGlobal = false;
        }
    }
}

ValidationResult semanticValidate(const Document& document)
{
    SemanticValidator validator(document);

    validator.validateGlobal();

    return validator.result;
}

} // namespace validation
} // namespace mnx
