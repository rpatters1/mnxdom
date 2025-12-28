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

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "BaseTypes.h"
#include "Global.h"
#include "Layout.h"

namespace mnx::util {

/// @brief Identifies a specific staff within a specific part.
///
/// A StaffKey uniquely identifies a part staff by pairing the part ID with
/// the 1-based staff index within that part. It is used to reason about
/// layout–part correspondence when validating or selecting conventional
/// score layouts.
struct StaffKey
{
    /// @brief The ID of the part.
    std::string partId;

    /// @brief The 1-based staff number within the part.
    int staffNo = 1;

    /// @brief Equality comparison.
    ///
    /// Two StaffKey values are equal if they refer to the same part ID
    /// and the same staff number within that part.
    bool operator==(const StaffKey& o) const noexcept
    {
        return staffNo == o.staffNo && partId == o.partId;
    }
};

/// @brief Hash functor for StaffKey.
///
/// Enables StaffKey to be used as a key in unordered associative containers.
struct StaffKeyHash
{
    /// @brief Computes a hash value for a StaffKey.
    ///
    /// The hash combines the part ID and staff number. It is intended for
    /// small collections and prioritizes simplicity over cryptographic strength.
    size_t operator()(const StaffKey& k) const noexcept
    {
        size_t h1 = std::hash<std::string>{}(k.partId);
        size_t h2 = std::hash<int>{}(k.staffNo);
        return h1 ^ (h2 + 0x9e3779b9u + (h1 << 6) + (h1 >> 2));
    }
};

/// @brief Analyzes a single layout staff for semantic voice usage.
///
/// For each distinct StaffKey (partId + staffNo) referenced by this staff:
/// - The StaffKey may appear exactly once (voice optional).
/// - If it appears multiple times, then every occurrence must specify
///   a non-empty voice, and all voice values must be distinct.
///
/// Multiple parts and multiple part staves on a single layout staff are permitted.
///
/// @param staff The layout staff to analyze.
/// @return A set of StaffKey values if the staff is semantically valid;
///         std::nullopt otherwise.
[[nodiscard]] inline std::optional<std::unordered_set<StaffKey, StaffKeyHash>>
analyzeLayoutStaffVoices(const layout::Staff& staff)
{
    const auto sources = staff.sources();
    if (sources.empty()) {
        return std::nullopt;
    }

    struct KeyState
    {
        size_t count = 0;
        bool anyMissingVoice = false;
        std::unordered_set<std::string> voices;
    };

    std::unordered_map<StaffKey, KeyState, StaffKeyHash> stateByKey;
    stateByKey.reserve(sources.size());

    for (const auto src : sources) {
        const std::string partId = src.part();
        if (partId.empty()) {
            return std::nullopt;
        }

        const StaffKey key{partId, src.staff()};
        auto& st = stateByKey[key];
        ++st.count;

        const auto v = src.voice();
        if (!v) {
            st.anyMissingVoice = true;
            continue;
        }

        if (v->empty()) {
            return std::nullopt;
        }

        if (!st.voices.emplace(*v).second) {
            return std::nullopt; // duplicate voice for same StaffKey
        }
    }

    // Enforce per-StaffKey semantic rule.
    for (const auto& kv : stateByKey) {
        const KeyState& st = kv.second;
        if (st.count > 1) {
            if (st.anyMissingVoice || st.voices.size() != st.count) {
                return std::nullopt;
            }
        }
    }

    std::unordered_set<StaffKey, StaffKeyHash> result;
    result.reserve(stateByKey.size());
    for (const auto& kv : stateByKey) {
        result.insert(kv.first);
    }

    return result;
}

/// @brief Flattens layout content into a linear sequence of staves.
///
/// Traverses a layout content array depth-first, recursively expanding groups
/// and appending staff elements in encounter order. The resulting sequence
/// preserves the visual staff order implied by the layout.
///
/// @param content The layout content array to traverse.
/// @return A vector of layout::Staff elements in encounter order if traversal
///         succeeds; std::nullopt if an unsupported content element is encountered.
[[nodiscard]] inline std::optional<std::vector<layout::Staff>>
flattenLayoutStaves(const ContentArray& content)
{
    std::vector<layout::Staff> result;
    result.reserve(content.size()); // lower bound; groups may expand further

    for (auto elem : content) {
        if (elem.type() == layout::Group::ContentTypeValue) {
            layout::Group g = elem.get<layout::Group>();
            auto nested = flattenLayoutStaves(g.content());
            if (!nested) {
                return std::nullopt;
            }
            result.insert(result.end(), nested->begin(), nested->end());
        } else if (elem.type() == layout::Staff::ContentTypeValue) {
            result.push_back(elem.get<layout::Staff>());
        } else {
            return std::nullopt;
        }
    }

    return result;
}

} // namespace mnx::util
