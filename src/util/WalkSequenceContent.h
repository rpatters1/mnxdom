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

#include <functional>

#include "Sequence.h"

namespace mnx::util {

/// @brief Traversal context passed through walkSequenceContent.
struct SequenceWalkContext
{
    FractionValue elapsedTime { 0 };    ///< current elapsed time.
    FractionValue timeRatio { 1 };      ///< accumulated tuplet time ratio.
    bool inGrace { false };             //< true while descending through grace note content
};

/// @brief Control flow returned by hooks invoked during walkSequenceContent.
enum class SequenceWalkControl
{
    Continue,       ///< Continue traversal normally.
    SkipChildren,   ///< Do not recurse into this item's children (if any).
    Stop            ///< Stop traversal immediately; walkSequenceContent returns false.
};

/// @brief Hook set for walkSequenceContent.
struct SequenceWalkHooks
{
    /// @brief Called for every content object before recursion or time advancement.
    std::function<SequenceWalkControl(const ContentObject& item,
                                      SequenceWalkContext& ctx)> onItem;

    /// @brief Called for events with computed timing.
    std::function<bool(const sequence::Event& event,
                       const FractionValue& startDuration,
                       const FractionValue& actualDuration,
                       SequenceWalkContext& ctx)> onEvent;

    /// @brief Called for every content object after recursion / time advancement.
    std::function<void(const ContentObject& item,
                       SequenceWalkContext& ctx)> onAfterItem;
};

/// @brief Walk an MNX sequence content tree, applying canonical timing semantics.
/// @param sequence The sequence traverse.
/// @param hooks Optional callbacks to observe/handle items.
/// @param ctx Optional in/out context (elapsed time and time ratio).
/// @return true if traversal completed; false if aborted by a hook.
inline bool walkSequenceContent(Sequence sequence,
                                const SequenceWalkHooks& hooks,
                                SequenceWalkContext* ctx = nullptr)
{
    SequenceWalkContext localCtx;
    SequenceWalkContext& c = ctx ? *ctx : localCtx;

    auto walkImpl = [&](ContentArray current,
                        SequenceWalkContext& ctxRef,
                        auto&& self) -> bool
    {
        for (const auto item : current) {
            bool allowChildren = true;

            if (hooks.onItem) {
                const SequenceWalkControl control = hooks.onItem(item, ctxRef);
                if (control == SequenceWalkControl::Stop) {
                    return false;
                }
                if (control == SequenceWalkControl::SkipChildren) {
                    allowChildren = false;
                }
            }

            if (item.type() == sequence::Event::ContentTypeValue) {
                const auto event = item.get<sequence::Event>();
                const auto start = ctxRef.elapsedTime;
                const auto actualDuration = event.calcDuration() * ctxRef.timeRatio;
                if (hooks.onEvent) {
                    if (!hooks.onEvent(event, start, actualDuration, ctxRef)) {
                        return false;
                    }
                }
                ctxRef.elapsedTime += actualDuration;
            } else if (item.type() == sequence::Grace::ContentTypeValue) {
                if (allowChildren) {
                    const auto grace = item.get<sequence::Grace>();
                    SequenceWalkContext child = ctxRef;
                    child.timeRatio = 0;
                    child.inGrace = true;
                    if (!self(grace.content(), child, self)) {
                        return false;
                    }
                    // Grace is time-neutral: elapsedTime does not change.
                }
            } else if (item.type() == sequence::Tuplet::ContentTypeValue) {
                const auto tuplet = item.get<sequence::Tuplet>();
                if (allowChildren) {
                    SequenceWalkContext child = ctxRef;
                    child.timeRatio = ctxRef.timeRatio * tuplet.ratio();
                    if (!self(tuplet.content(), child, self)) {
                        return false;
                    }
                    // Propagate elapsedTime back to the parent context.
                    ctxRef.elapsedTime = child.elapsedTime;
                } else {
                    // Children skipped: assume well-formed tuplet content.
                    ctxRef.elapsedTime += tuplet.outer() * ctxRef.timeRatio;
                }
            } else if (item.type() == sequence::MultiNoteTremolo::ContentTypeValue) {
                const auto tremolo = item.get<sequence::MultiNoteTremolo>();
                /// @todo: MNX tremolo durations.
                // Currently, inner tremolo notes are treated as time-neutral and only
                // tremolo.outer() * timeRatio advances elapsedTime. Once the MNX spec
                // clarifies per-note duration semantics for multi-note tremolos, this
                // iterator should be updated so each event gets an appropriate share of
                // the tremolo duration pie.
                if (allowChildren) {
                    SequenceWalkContext child = ctxRef;
                    child.timeRatio = 0;
                    if (!self(tremolo.content(), child, self)) {
                        return false;
                    }
                }
                ctxRef.elapsedTime += tremolo.outer() * ctxRef.timeRatio;
            } else if (item.type() == sequence::Space::ContentTypeValue) {
                const auto space = item.get<sequence::Space>();
                ctxRef.elapsedTime += space.duration() * ctxRef.timeRatio;
            }

            if (hooks.onAfterItem) {
                hooks.onAfterItem(item, ctxRef);
            }
        }

        return true;
    };

    return walkImpl(sequence.content(), c, walkImpl);
}

/// @brief Iterate all the events in a sequence content tree in order as they come.
/// @param sequence The sequence to traverse.
/// @param iterator Callback function invoked for each event.
///     The callback must have signature:
///     `bool(sequence::Event event,
///           FractionValue startDuration,
///           FractionValue actualDuration)`.
///     - `event`: the current event in the sequence.
///     - `startDuration`: total elapsed metric time before this event.
///     - `actualDuration`: the event’s real performed duration.
///     - return @c true to continue iterating.
/// @return true if iteration completed without interruption; false if it exited early.
/// @todo Multi-note tremolos are currently treated as a span whose outer()
///       value advances time. Inner tremolo notes have zero actual duration until
///       the MNX spec clarifies how their durations should be distributed.
inline bool iterateSequenceEvents(Sequence sequence,
                                  std::function<bool(const sequence::Event& event,
                                                     const FractionValue& startDuration,
                                                     const FractionValue& actualDuration)> iterator)
{
    SequenceWalkHooks hooks;
    hooks.onEvent = [&](const sequence::Event& event,
                        const FractionValue& startDuration,
                        const FractionValue& actualDuration,
                        SequenceWalkContext&) -> bool
    {
        // Preserve the public callback signature: pass Event by value.
        return iterator(event, startDuration, actualDuration);
    };

    return walkSequenceContent(sequence, hooks);
}

} // namespace mnx::util
