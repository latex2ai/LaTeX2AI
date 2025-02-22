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


#ifndef L2A_ANNOTATOR_H_
#define L2A_ANNOTATOR_H_


#include "l2a_global.h"
#include "l2a_string_functions.h"
#include "l2a_suites.h"

#include <map>

// Forward declaration.
namespace L2A
{
    class Item;
}


namespace L2A
{
    class Annotator
    {
       public:
        /**	Constructor. Create the annotator.
         */
        Annotator(SPInterfaceMessage* message);

        /**	Destructor.
         */
        ~Annotator() {};

        /**
         * \brief This method is called when the art selection changed. If the annotator is active, the items are
         * reloaded.
         */
        void ArtSelectionChanged();

        /**
         * \brief This method is called when the art selection changed. If the annotator is active, the items are
         * reloaded.
         */
        std::pair<int, int> NumberActiveArt() const;

        /**
         * \brief Return if the annotator is active or not.
         */
        bool IsActive() const;

        /**
         * \brief Set the annotator including invalidating the annotator and reseting the item position tracker vector.
         */
        void SetAnnotatorWithInvalidateBounds(bool active);

        /**
         * \brief Set the annotator active.
         */
        void SetAnnotatorActive();

        /**
         * \brief Set the annotator inactive.
         */
        void SetAnnotatorInactive();

        /**
         * \brief Check if any art items are underneath the cursor.
         */
        bool CheckForArtHit(AIToolMessage* message);

        /**
         * \brief Return true if an item is hit.
         */
        bool IsArtHit() const { return cursor_item_ != nullptr; }

        /**
         * \brief Return true hit item.
         */
        AIArtHandle GetArtHit() const
        {
            L2A::GlobalMutable().logger_.push_back(
                ai::UnicodeString("L2A::Annotator::GetArtHit Line ") + L2A::UTIL::IntegerToString(__LINE__));


            return cursor_item_;
        }

        /**
         * \brief Draw the boundaries of the items.
         */
        void Draw(AIAnnotatorMessage* message) const;

        /**
         * \brief Invalidate the annotation.
         */
        void InvalAnnotation(const AIRealRect& artwork_bounds) const;

        /**
         * \brief Invalidate the entire document view bounds.
         */
        void InvalAnnotation() const;

       private:
        /**
         * \brief Set the annotator inactive.
         */
        void SetAnnotator(bool active);

       private:
        //! Handle for the annotator added by this plug-in.
        AIAnnotatorHandle annotator_handle_;

        //! Item the cursor is over.
        AIArtHandle cursor_item_;

        //! Vector of items. The items are stored in pairs, where the second pair entry are all positions of the
        //! bounding box.
        std::vector<std::pair<L2A::Item, std::map<PlaceAlignment, AIRealPoint>>> item_vector_;
    };
}  // namespace L2A

#endif
