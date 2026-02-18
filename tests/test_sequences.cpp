/*
 * Copyright (C) 2024, Robert Patterson
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
#include "gtest/gtest.h"
#include "mnxdom.h"
#include "test_utils.h"

using namespace mnx;

void checkSequence(const part::Measure& measure, size_t contentIdx, int expectedTranKeyFifths,
    const sequence::Pitch::Required& expected1stTransPitch)
{
    ASSERT_FALSE(measure.sequences().empty());
    auto sequence = measure.sequences()[0];
    ASSERT_GT(sequence.content().size(), contentIdx);
    auto item = sequence.content()[contentIdx];
    ASSERT_EQ(item.type(), sequence::Event::ContentTypeValue);
    auto event = item.get<sequence::Event>();
    ASSERT_TRUE(event.notes());
    ASSERT_FALSE(event.notes()->empty());
    auto note = event.notes()->at(0);
    auto firstTransPitch = note.pitch().calcTransposed();
    EXPECT_EQ(firstTransPitch.step, expected1stTransPitch.step) << "transposed pitch step does not match expected value";
    EXPECT_EQ(firstTransPitch.octave, expected1stTransPitch.octave) << "transposed pitch octave does not match expected value";
    EXPECT_EQ(firstTransPitch.alter, expected1stTransPitch.alter) << "transposed pitch alter does not match expected value";

    auto globalMeasure = measure.getGlobalMeasure();
    auto part = measure.getEnclosingElement<Part>();
    ASSERT_TRUE(part);
    auto expectedTranKeySig = KeySignature::make(expectedTranKeyFifths);
    if (auto partTran = part->transposition()) {
        EXPECT_EQ(partTran->calcTransposedKey(globalMeasure.calcCurrentKeyFields()).fifths, expectedTranKeySig.fifths) << "transposed key does not match expected value";
    } else {
        EXPECT_EQ(globalMeasure.calcCurrentKeyFields().fifths, expectedTranKeySig.fifths) << "concert key does not match expected value";
    }
}

TEST(TestSequences, TransposedPitches)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "key7-7-wrapped.json";
    auto doc = mnx::Document::create(inputPath);
    fullValidate(doc, inputPath);

    ASSERT_GE(doc.parts().size(), 4);
    ASSERT_GE(doc.global().measures().size(), 2);

    {
        auto part = doc.parts()[0];
        checkSequence(part.measures()[0], 0, +7, { NoteStep::G, 4, +1 });
        checkSequence(part.measures()[1], 0, -7, { NoteStep::C, 5, -1 });
    }
    {
        auto part = doc.parts()[1];
        checkSequence(part.measures()[0], 0, -3, { NoteStep::G, 4, 0 });
        checkSequence(part.measures()[1], 0, -5, { NoteStep::B, 4, -1 });
    }
    {
        auto part = doc.parts()[2];
        checkSequence(part.measures()[0], 0, +4, { NoteStep::A, 4, 0 });
        checkSequence(part.measures()[1], 0, +2, { NoteStep::D, 5, 0 });
    }
    {
        auto part = doc.parts()[3];
        checkSequence(part.measures()[0], 0, -2, { NoteStep::B, 4, -1 });
        checkSequence(part.measures()[1], 0, -4, { NoteStep::B, 4, -1 });
    }
}

TEST(TestSequences, TransposedPitchesNoWrap)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "key7-7.json";
    auto doc = mnx::Document::create(inputPath);
    fullValidate(doc, inputPath);

    ASSERT_GE(doc.parts().size(), 4);
    ASSERT_GE(doc.global().measures().size(), 2);

    {
        auto part = doc.parts()[0];
        checkSequence(part.measures()[0], 0, +7, { NoteStep::C, 5, +1 });
        checkSequence(part.measures()[1], 0, -7, { NoteStep::C, 5, -1 });
    }
    {
        auto part = doc.parts()[1];
        checkSequence(part.measures()[0], 2, +9, { NoteStep::F, 5, +2 });
        checkSequence(part.measures()[1], 0, -5, { NoteStep::D, 5, -1 });
    }
    {
        auto part = doc.parts()[2];
        checkSequence(part.measures()[0], 0, +4, { NoteStep::E, 5, 0 });
        checkSequence(part.measures()[1], 0, -10, { NoteStep::E, 5, -2 });
    }
    {
        auto part = doc.parts()[3];
        checkSequence(part.measures()[0], 2, -2, { NoteStep::D, 6, 0 });
        checkSequence(part.measures()[1], 0, -4, { NoteStep::A, 5, -1 });
    }
}

TEST(TestSequences, WalkFullMeasureRest)
{
    auto doc = Document();
    auto globalMeasure = doc.global().measures().append();
    globalMeasure.ensure_time(4, TimeSignatureUnit::Quarter);
    auto part = doc.parts().append();
    auto measure = part.create_measures().append();
    auto sequence = measure.sequences().append();
    auto fullMeasure = sequence.ensure_fullMeasure();

    int fullMeasureCalls = 0;
    FractionValue fullMeasureStart = 0;
    FractionValue fullMeasureDuration = 0;
    util::SequenceWalkContext ctx;
    util::SequenceWalkHooks hooks;
    hooks.onFullMeasure = [&](const Sequence& callbackSequence,
                              const sequence::FullMeasureRest& callbackFullMeasure,
                              const FractionValue& startDuration,
                              const FractionValue& actualDuration,
                              util::SequenceWalkContext&) -> bool {
        ++fullMeasureCalls;
        EXPECT_EQ(callbackSequence.pointer(), sequence.pointer());
        EXPECT_EQ(callbackFullMeasure.pointer(), fullMeasure.pointer());
        fullMeasureStart = startDuration;
        fullMeasureDuration = actualDuration;
        return true;
    };

    const bool walked = util::walkSequenceContent(sequence, hooks, &ctx);
    ASSERT_TRUE(walked);
    EXPECT_EQ(fullMeasureCalls, 1);
    EXPECT_EQ(fullMeasureStart, FractionValue(0));
    EXPECT_EQ(fullMeasureDuration, static_cast<FractionValue>(globalMeasure.time().value()));
    EXPECT_EQ(ctx.elapsedTime, fullMeasureDuration);
}
