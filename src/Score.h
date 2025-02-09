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

namespace mnx {

/**
 * @class MultimeasureRest
 * @brief Represents a multimeasure rest in a score.
 */
class MultimeasureRest : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;
    /// note that after adding a new MultimeasureRest, the caller *must* provide start & duration for it to validate

    MNX_REQUIRED_PROPERTY(int, duration);           ///< the number of measures in the multimeasure rest
    MNX_OPTIONAL_PROPERTY(std::string, label);      ///< the label to place on the multimeasure rest, if provided.
    MNX_REQUIRED_PROPERTY(int, start);              ///< the start measure of the multimeasure rest
};

/**
 * @class SystemLayoutChange
 * @brief Represents a system layout change in a score
 */
class SystemLayoutChange : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;
    /// note that after adding a new MultimeasureRest, the caller *must* provide start & duration for it to validate

    MNX_REQUIRED_PROPERTY(std::string, layout);     ///< Layout id, referring to an element in the root-level layouts array.
    /// @todo location
};

/**
 * @class ScoreSystem
 * @brief Represents a system on a page in a score.
 */
class ScoreSystem : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;
    /// note that after adding a new ScorePage, the caller *must* provide a start measure value for it to validate

    MNX_OPTIONAL_PROPERTY(std::string, layout);     ///< Layout id, referring to an element in the root-level layouts array.
    MNX_OPTIONAL_CHILD(Array<SystemLayoutChange>, layoutChanges); ///< layout changes in the system (e.g., for changes in stem direction)
    MNX_REQUIRED_PROPERTY(int, measure);            ///< The first measure in the system
};

/**
 * @class ScorePage
 * @brief Represents a page in a score.
 */
class ScorePage : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    /// @brief Creates a new Global class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding the new array.
    ScorePage(Base& parent, const std::string_view& key)
        : ArrayElementObject(parent, key)
    {
        // required children
        create_systems();
    }

    MNX_OPTIONAL_PROPERTY(std::string, layout);         ///< Layout id, referring to an element in the root-level layouts array.
    MNX_REQUIRED_CHILD(Array<ScoreSystem>, systems);    ///< and array systems on the page.
};

/**
 * @class Score
 * @brief Represents an element in the scores section of an MNX document.
 */
class Score : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    /// note that after adding a new Score, the caller *must* provide a name for it to validate

    MNX_OPTIONAL_PROPERTY(std::string, layout);                         ///< Layout id, referring to an element in the root-level layouts array.
    MNX_OPTIONAL_CHILD(Array<MultimeasureRest>, multimeasureRests);     ///< List of multimeasure rests in the score.
    MNX_REQUIRED_PROPERTY(std::string, name);                           ///< Required name, such as "Flute 1" or "Full Score".
    MNX_OPTIONAL_CHILD(Array<ScorePage>, pages);                        ///< An optional list of pages.
};

} // namespace mnx