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
 * \brief A object that represents one l2a item in ai.
 */


#ifndef L2A_ITEM_H_
#define L2A_ITEM_H_


#include "l2a_latex.h"
#include "l2a_property.h"
#include "l2a_suites.h"

#include <map>


// Forward declaration
struct AIAnnotatorMessage;


// Forward declaration.
namespace L2A
{
    namespace TEST
    {
        namespace UTIL
        {
            class UnitTest;
        }
        void TestFramework(L2A::TEST::UTIL::UnitTest& ut);
        namespace PRIVATE
        {
            void CheckItems(L2A::TEST::UTIL::UnitTest& ut);
        }
    }  // namespace TEST
}  // namespace L2A


namespace L2A
{
    /**
     * \brief Container to store data returned from an item change operation
     */
    struct ItemChangeResult
    {
        /**
         * \brief Enum to manage the return values of the change function of the item
         */
        enum class Result
        {
            //! Everything worked, the item was changed
            ok,
            //! There was a LaTeX error user has to be asked for the input again
            latex_error,
            //! Not changed, and no new input wanted
            cancel
        };

        //! Flag with the result value
        Result result_;

        //! Container with the results from the latex creation
        L2A::LATEX::LatexCreationResult latex_creation_result_;
    };


    /**
     * \brief Class that represents one LaTeX2AI item in AI. It has methods for creating and replacing the LaTeX
     *        document in AI.
     */
    class Item
    {
        // Define the friend functions for testing.
        friend void L2A::TEST::TestFramework(L2A::TEST::UTIL::UnitTest& ut);
        friend void L2A::TEST::PRIVATE::CheckItems(L2A::TEST::UTIL::UnitTest& ut);

       public:
        /**
         * \brief Create a new L2AItem from a position the user clicked in the document and a given property object
         * @param position AIRealPoint of the cursor in the document
         * @param property Property of the item, has to include the saved pdf file
         * @param created_pdf_file Path to the (existing) pdf file
         */
        Item(const AIRealPoint& position, const L2A::Property& property, const ai::FilePath& created_pdf_file);

        /**
         * \brief Create the object from an existing placed item
         * @param placed_item AIArtHandle of a placed item in the document.
         */
        Item(const AIArtHandle& placed_item_handle);

        /**
         * \brief Change the l2a item
         * @return Return the result of the item change operation
         */
        ItemChangeResult Change(const ai::UnicodeString& form_return_value, L2A::Property& new_property);

        /**
         * \brief Get a non-const reference to the property of this item.
         */
        L2A::Property& GetPropertyMutable() { return property_; }

        /**
         * \brief Get a const reference to the property of this item.
         */
        const L2A::Property& GetProperty() const { return property_; }

        /**
         * \brief Get a const reference to a placed item.
         */
        const AIArtHandle& GetPlacedItem() const { return placed_item_; }

        /**
         * \brief Get a mutable reference to a placed item.
         */
        AIArtHandle& GetPlacedItemMutable() { return placed_item_; }

        /**
         * \brief Redo the boundary box of the item.
         */
        void RedoBoundary();

        /**
         * \brief Set displayed name of the placed item in iIllustrator and set the property data as note.
         */
        void SetNoteAndName() const;

        /**
         * \brief Get the name of the PDF file for this item.
         */
        ai::FilePath GetPDFPath() const;

        /**
         * \brief Create the encoded PDF file.
         */
        void SaveEncodedPDFFile(const ai::FilePath& pdf_path) const;

        /**
         * \brief Get the name of the item in Illustrator.
         */
        ai::UnicodeString GetAIName() const;

        /**
         * \brief Get the position of the item with the current property.
         */
        AIRealPoint GetPosition() const;

        /**
         * \brief Get the positions of multiple points on the item.
         */
        std::vector<AIRealPoint> GetPosition(const std::vector<PlaceAlignment>& placements) const;

        /**
         * \brief Draw the boundary of the placed item in the document.
         */
        void Draw(AIAnnotatorMessage* message, const std::map<PlaceAlignment, AIRealPoint>& item_boundaries) const;

        bool IsValid() const
        {
            auto is_valid = sAIArt->ValidArt(this->placed_item_, true);
            return is_valid;
        }

        /**
         * \brief Check if the item is of diamond shape.
         */
        bool IsDiamond() const;

        /**
         * \brief Check if the item is stretched.
         */
        bool IsStretched() const;

       private:
        /**
         * \brief Move the placed item to a desired position.
         */
        void MoveItem(const AIRealPoint& position_item);

        /**
         * \brief Get the angle of the item along the x1 or x2 axis (director is 0 for x1 and 1 for x2).
         */
        AIReal GetAngle(unsigned short director = 0) const;

        /**
         * \brief Check if the item is rotated.
         */
        bool IsRotated() const;

        /**
         * \brief Get the stretch of the item in a direction.
         */
        AIReal GetStretch(unsigned short director = 0) const;

       private:
        //! Properties of this item.
        L2A::Property property_;

        //! Pointer to the art handle in AI.
        AIArtHandle placed_item_;
    };

    /**
     * \brief Flag for the redo items function
     */
    enum class RedoItemsOption
    {
        latex,
        bounding_box
    };

    /**
     * \brief Redo all items. Give the user the option to chose what to redo.
     */
    void RedoItems(std::vector<AIArtHandle>& items, const RedoItemsOption& redo_option);

    /**
     * \brief Redo the LaTeX code for all items in the vector.
     */
    bool RedoLaTeXItems(std::vector<L2A::Item>& l2a_items);

    /**
     * \brief Check if the pdf files of the items are stored and linked correctly.
     */
    void CheckItemDataStructure();

}  // namespace L2A
#endif
