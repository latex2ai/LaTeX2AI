// -----------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2020-2024 Ivo Steinbrecher
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// -----------------------------------------------------------------------------

/**
 * \brief Object that handles everython annotator related.
 */


#include "IllustratorSDK.h"

#include "l2a_annotator.h"

#include "l2a_ai_functions.h"
#include "l2a_error.h"
#include "l2a_global.h"
#include "l2a_item.h"


/**
 *
 */
L2A::Annotator::Annotator(SPInterfaceMessage* message) : cursor_item_(nullptr)
{
    ASErr error = kNoErr;

    // Create the annotator.
    error = sAIAnnotator->AddAnnotator(message->d.self, "LaTeX2AI Annotator", &annotator_handle_);
    l2a_check_ai_error(error);

    // At the beginning, the annotator has to be inactive. This has to be done with the SetAnnotator method as that one
    // does not invalidate the boundaries, which at the point of the creation of this object would not work.
    SetAnnotator(false);
}


std::pair<int, int> L2A::Annotator::NumberActiveArt() const
{
    int n_valid = 0;
    for (const auto& item : item_vector_)
    {
        if (item.first.IsValid())
        {
            n_valid += 1;
        }
    }
    return {item_vector_.size(), n_valid};
}

/**
 *
 */
void L2A::Annotator::ArtSelectionChanged()
{
    L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("L2A::Annotator::ArtSelectionChanged Line ") + L2A::UTIL::IntegerToString(__LINE__));

    // Reset the item vector.
    item_vector_.clear();

    // Only do something if the annotator is active.
    if (!IsActive())
    {
        L2A::GlobalMutable().logger_.push_back(
            ai::UnicodeString("  IsActive: ") + L2A::UTIL::IntegerToString(__LINE__));
        return;
    }
    else
    {
        // Get all l2a items in the document.
        std::vector<AIArtHandle> all_items;
        L2A::AI::GetDocumentItems(all_items, L2A::AI::SelectionState::all);
        for (auto& item : all_items)
        {
            L2A::GlobalMutable().logger_.push_back(
                ai::UnicodeString("  Add item: ") + L2A::UTIL::IntegerToString(__LINE__));

            // Create item object.
            L2A::Item new_item(item);

            // Get all coordinates of the item.
            std::vector<PlaceAlignment> placements;
            placements.push_back(PlaceAlignment::kTopLeft);
            placements.push_back(PlaceAlignment::kTopMid);
            placements.push_back(PlaceAlignment::kTopRight);
            placements.push_back(PlaceAlignment::kMidLeft);
            placements.push_back(PlaceAlignment::kMidMid);
            placements.push_back(PlaceAlignment::kMidRight);
            placements.push_back(PlaceAlignment::kBotLeft);
            placements.push_back(PlaceAlignment::kBotMid);
            placements.push_back(PlaceAlignment::kBotRight);
            std::vector<AIRealPoint> item_points = new_item.GetPosition(placements);

            // Fill up the map.
            std::map<PlaceAlignment, AIRealPoint> item_boundaries;
            for (unsigned int i_placement = 0; i_placement < placements.size(); i_placement++)
                item_boundaries[placements[i_placement]] = item_points[i_placement];

            // Add to the item vetor.
            item_vector_.push_back(std::make_pair(new_item, item_boundaries));
        }
    }
}

/**
 *
 */
bool L2A::Annotator::IsActive() const
{
    ASErr error = kNoErr;
    AIBoolean is_active;
    error = sAIAnnotator->GetAnnotatorActive(annotator_handle_, &is_active);
    l2a_check_ai_error(error);
    return is_active;
}

/**
 *
 */
void L2A::Annotator::SetAnnotatorWithInvalidateBounds(bool active)
{
    SetAnnotator(active);

    // This only makes sense if an Illustrator document is opened.
    if (L2A::AI::GetDocumentCount() > 0)
    {
        ArtSelectionChanged();
        InvalAnnotation();
    }
}

/**
 *
 */
void L2A::Annotator::SetAnnotatorActive()
{
    L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("L2A::Annotator::SetAnnotatorActive Line ") + L2A::UTIL::IntegerToString(__LINE__));

    SetAnnotatorWithInvalidateBounds(true);
}

/**
 *
 */
void L2A::Annotator::SetAnnotatorInactive()
{
    L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("L2A::Annotator::SetAnnotatorInactive Line ") + L2A::UTIL::IntegerToString(__LINE__));

    SetAnnotatorWithInvalidateBounds(false);
}

/**
 *
 */
void L2A::Annotator::SetAnnotator(bool active)
{
    ASErr error = sAIAnnotator->SetAnnotatorActive(annotator_handle_, active);
    l2a_check_ai_error(error);
}

/**
 *
 */
bool L2A::Annotator::CheckForArtHit(AIToolMessage* message)
{
    L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("L2A::Annotator::CheckForArtHit Line ") + L2A::UTIL::IntegerToString(__LINE__));

    ASErr result = kNoErr;

    // This function can only be called if the annotator is active.
    if (!IsActive()) l2a_error("Annotator has to be active.");

    // Check if cursor is over any art.
    AIHitRef hitRef = nullptr;
    AIToolHitData toolHitData;
    result = sAIHitTest->HitTest(nullptr, &message->cursor, kAllHitRequest, &hitRef);
    l2a_check_ai_error(result);
    result = sAIHitTest->GetHitData(hitRef, &toolHitData);
    l2a_check_ai_error(result);

    // Check if the item is a L2AItem.
    if (toolHitData.hit && toolHitData.object != nullptr && L2A::AI::IsL2AItem(toolHitData.object))
    {
        // Set the last art item to hit.
        cursor_item_ = toolHitData.object;
        return true;
    }
    else
    {
        cursor_item_ = nullptr;
        return false;
    }
}

/**
 *
 */
void L2A::Annotator::Draw(AIAnnotatorMessage* message) const
{
    L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("L2A::Annotator::Draw Line ") + L2A::UTIL::IntegerToString(__LINE__));

    // This can only be called when the annotator is active.
    if (!IsActive()) l2a_error("The annotator has to be active.");

    // Check how many items are present in this document
    std::vector<AIArtHandle> l2a_items;
    L2A::AI::GetDocumentItems(l2a_items, L2A::AI::SelectionState::all);
    L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("  number items in document: ") + L2A::UTIL::IntegerToString(l2a_items.size()));

    L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("  number items in item_vector_: ") + L2A::UTIL::IntegerToString(item_vector_.size()));

    // Loop over items and draw boundary.
    int counter = 0;
    for (const auto& item : item_vector_)
    {
        L2A::GlobalMutable().logger_.push_back(
            ai::UnicodeString("  loop iteration: ") + L2A::UTIL::IntegerToString(counter));
        std::get<0>(item).Draw(message, std::get<1>(item));
        counter++;
    }
}

/**
 *
 */
void L2A::Annotator::InvalAnnotation(const AIRealRect& artwork_bounds) const
{
    // Get the rectangle to invalidate.
    AIRect inval_rect = L2A::AI::ArtworkBoundsToViewBounds(artwork_bounds);

    // Invalidate the rect bounds so it is redrawn.
    AIErr result = sAIAnnotator->InvalAnnotationRect(nullptr, &inval_rect);
    l2a_check_ai_error(result);
}

/**
 *
 */
void L2A::Annotator::InvalAnnotation() const
{
    // Get the bounds of the current document view.
    AIRealRect view_bounds = {0, 0, 0, 0};
    AIErr result = sAIDocumentView->GetDocumentViewBounds(nullptr, &view_bounds);
    l2a_check_ai_error(result);

    // Invalidate the rect bounds so it is redrawn.
    InvalAnnotation(view_bounds);
}
