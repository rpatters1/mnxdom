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
#include <fstream>

#include "nlohmann/json-schema.hpp"
#include "mnx_schema.xxd"

#include "mnxdom.h"

namespace mnx {

static const std::string_view MNX_SCHEMA(reinterpret_cast<const char*>(mnx_schema_json), mnx_schema_json_len);

// ********************
// ***** Document *****
// ********************

std::optional<std::string> Document::validate(const std::optional<std::string>& jsonSchema) const
{
    try {
        // Load JSON schema
        json schemaJson = json::parse(jsonSchema.value_or(std::string(MNX_SCHEMA)));
        nlohmann::json_schema::json_validator validator;
        validator.set_root_schema(schemaJson);
        validator.validate(*root());
    } catch (const std::invalid_argument& e) {
        return e.what();
    }
    return std::nullopt;
}

// ***************************
// ***** global::Measure *****
// ***************************

BarlineType global::Measure::calcBarlineType() const
{
    if (const auto thisBarline = barline()) {
        return thisBarline.value().type();
    }
    const auto parentArray = parent<Array<global::Measure>>();
    size_t arrayIndex = calcArrayIndex();
    return (arrayIndex + 1) == parentArray.size() ? BarlineType::Final : BarlineType::Regular;
}

int global::Measure::calcMeasureIndex() const
{
    if (auto thisIndex = index()) {
        return thisIndex.value();
    }
    size_t arrayIndex = calcArrayIndex();
    if (arrayIndex == 0) return 1;
    const auto parentArray = parent<Array<global::Measure>>();
    const auto prev = parentArray[arrayIndex - 1];
    const auto prevIndex = prev.index();
    return prevIndex.has_value() ? *prevIndex : prev.calcMeasureIndex();
}

} // namespace mnx