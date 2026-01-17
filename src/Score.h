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
#include "CommonClasses.h"

namespace mnx {

/**
 * @namespace mnx::score
 * @brief classes related to the root scores array
 */
namespace score {

/**
 * @class MultimeasureRest
 * @brief Represents a multimeasure rest in a score.
 */
#ifndef DOXYGEN_SHOULD_IGNORE_THIS
#define MNX_SCORE_MULTIMEASURE_REST_FIELDS(M) \
    M(int, startMeasure), \
    M(int, numMeasures)
#define MNX_SCORE_MULTIMEASURE_REST_CTOR_ARGS \
    MNX_FIELDS_AS_PARAMS(MNX_SCORE_MULTIMEASURE_REST_FIELDS)
#endif // DOXYGEN_SHOULD_IGNORE_THIS
class MultimeasureRest : public ArrayElementObject
{
public:
    /// @brief initializer class for #MultimeasureRest
    struct Required
    {
        int startMeasure{}; ///< the measure index of the first measure in the multimeasure rest
        int numMeasures{};  ///< the number of measures in the multimeasure rest
    };

    /// @brief Constructor for existing mm rests
    MultimeasureRest(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }
    
    /// @brief Creates a new MultimeasureRest class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param startMeasure The measure index of the first measure in the multimeasure rest
    /// @param numMeasures The number of measures in the multimeasure rest
    MultimeasureRest(Base& parent, std::string_view key, MNX_SCORE_MULTIMEASURE_REST_CTOR_ARGS)
        : ArrayElementObject(parent, key)
    {
        set_start(startMeasure);
        set_duration(numMeasures);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { start(), duration() }; }

    /// @brief Create a Required instance for #MultimeasureRest.
    static Required make(MNX_SCORE_MULTIMEASURE_REST_CTOR_ARGS) { return { startMeasure, numMeasures }; }

    MNX_REQUIRED_PROPERTY(int, duration);           ///< the number of measures in the multimeasure rest
    MNX_OPTIONAL_PROPERTY(std::string, label);      ///< the label to place on the multimeasure rest, if provided.
    MNX_REQUIRED_PROPERTY(int, start);              ///< the start measure of the multimeasure rest
};

/**
 * @class LayoutChange
 * @brief Represents a system layout change in a score
 */
#ifndef DOXYGEN_SHOULD_IGNORE_THIS
#define MNX_SCORE_LAYOUT_CHANGE_FIELDS(M) \
    M(const std::string&, layoutId), \
    M(int, measureId), \
    M(const FractionValue&, position)
#define MNX_SCORE_LAYOUT_CHANGE_CTOR_ARGS \
    MNX_FIELDS_AS_PARAMS(MNX_SCORE_LAYOUT_CHANGE_FIELDS)
#endif // DOXYGEN_SHOULD_IGNORE_THIS
class LayoutChange : public ArrayElementObject
{
public:
    /// @brief initializer class for #LayoutChange
    struct Required
    {
        std::string layoutId;     ///< the id of the layout to use for the layout change
        int measureId{};          ///< the measure index of the measure of the position
        FractionValue position{}; ///< the position of the LayoutChange within the measure
    };

    /// @brief Constructor for existing system layouts
    LayoutChange(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }
    
    /// @brief Creates a new LayoutChange class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param layoutId The id of the layout to use for the layout change
    /// @param measureId The measure index of the measure of the position
    /// @param position The position of the LayoutChange within the measure
    LayoutChange(Base& parent, std::string_view key, MNX_SCORE_LAYOUT_CHANGE_CTOR_ARGS)
        : ArrayElementObject(parent, key)
    {
        set_layout(layoutId);
        create_location(measureId, position);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { layout(), location().measure(), location().position().fraction() }; }

    /// @brief Create a Required instance for #LayoutChange.
    static Required make(MNX_SCORE_LAYOUT_CHANGE_CTOR_ARGS)
    { return { layoutId, measureId, position }; }

    MNX_REQUIRED_PROPERTY(std::string, layout);             ///< Layout id, referring to an element in the root-level layouts array.
    MNX_REQUIRED_CHILD(MeasureRhythmicPosition, location,
        MNX_FIELDS_AS_TUPLES(MNX_MEASURE_RHYTHMIC_POSITION_FIELDS)); ///< location where the new layout takes effect.
};

/**
 * @class System
 * @brief Represents a system on a page in a score.
 */
#ifndef DOXYGEN_SHOULD_IGNORE_THIS
#define MNX_SCORE_SYSTEM_FIELDS(M) \
    M(int, startMeasure)
#define MNX_SCORE_SYSTEM_CTOR_ARGS \
    MNX_FIELDS_AS_PARAMS(MNX_SCORE_SYSTEM_FIELDS)
#endif // DOXYGEN_SHOULD_IGNORE_THIS
class System : public ArrayElementObject
{
public:
    /// @brief initializer class for #System
    struct Required
    {
        int startMeasure{}; ///< the measure index of the first measure in the system
    };

    /// @brief Constructor for existing system layouts
    System(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }
    
    /// @brief Creates a new System class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param startMeasure The measure index of the first measure in the system
    System(Base& parent, std::string_view key, MNX_SCORE_SYSTEM_CTOR_ARGS)
        : ArrayElementObject(parent, key)
    {
        set_measure(startMeasure);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { measure() }; }

    /// @brief Create a Required instance for #System.
    static Required make(MNX_SCORE_SYSTEM_CTOR_ARGS) { return { startMeasure }; }

    MNX_OPTIONAL_PROPERTY(std::string, layout);     ///< Layout id, referring to an element in the root-level layouts array.
    MNX_OPTIONAL_CHILD(Array<LayoutChange>, layoutChanges); ///< layout changes in the system (e.g., for changes in stem direction)
    MNX_REQUIRED_PROPERTY(int, measure);            ///< The first measure in the system
};

/**
 * @class Page
 * @brief Represents a page in a score.
 */
class Page : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    /// @brief Creates a new Page class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Page(Base& parent, std::string_view key)
        : ArrayElementObject(parent, key)
    {
        // required children
        create_systems();
    }

    MNX_OPTIONAL_PROPERTY(std::string, layout);     ///< Layout id, referring to an element in the root-level layouts array.
    MNX_REQUIRED_CHILD(Array<System>, systems);     ///< and array systems on the page.
};

} // namespace score

/**
 * @class Score
 * @brief Represents an element in the scores section of an MNX document.
 */
#ifndef DOXYGEN_SHOULD_IGNORE_THIS
#define MNX_SCORE_SCORE_FIELDS(M) \
    M(const std::string&, scoreName)
#define MNX_SCORE_SCORE_CTOR_ARGS \
    MNX_FIELDS_AS_PARAMS(MNX_SCORE_SCORE_FIELDS)
#endif // DOXYGEN_SHOULD_IGNORE_THIS
class Score : public ArrayElementObject
{
public:
    /// @brief initializer class for #Score
    struct Required
    {
        std::string scoreName; ///< the name of the score to be created
    };

    /// @brief Constructor for existing system layouts
    Score(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }
    
    /// @brief Creates a new Score class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param scoreName The name of the score to be created
    Score(Base& parent, std::string_view key, MNX_SCORE_SCORE_CTOR_ARGS)
        : ArrayElementObject(parent, key)
    {
        set_name(scoreName);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { name() }; }

    /// @brief Create a Required instance for #Score.
    static Required make(MNX_SCORE_SCORE_CTOR_ARGS) { return { scoreName }; }

    MNX_OPTIONAL_PROPERTY(std::string, layout);                             ///< Layout id, referring to an element in the root-level layouts array.
    MNX_OPTIONAL_CHILD(Array<score::MultimeasureRest>, multimeasureRests);  ///< List of multimeasure rests in the score.
    MNX_REQUIRED_PROPERTY(std::string, name);                               ///< Required name, such as "Flute 1" or "Full Score".
    MNX_OPTIONAL_CHILD(Array<score::Page>, pages);                          ///< An optional list of pages.
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, useWritten, false);            ///< If this value is true, the score should be displayed transposed.
};

} // namespace mnx
