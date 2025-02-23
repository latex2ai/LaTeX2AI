// -----------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2020-2025 Ivo Steinbrecher
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
 * \brief Function wrappers for Illustrator functions.
 */


#ifndef L2A_AI_FUNCTIONS_H_
#define L2A_AI_FUNCTIONS_H_


#include "IllustratorSDK.h"

// Forward declarations.
namespace L2A
{
    class Property;
}


namespace L2A
{
    namespace AI
    {
        /**
         *\brief Enum for type of selected items.
         */
        enum SelectionState
        {
            //! All items.
            all,
            //! Only selected items.
            selected,
            //! Only deselected items.
            deselected
        };

        /**
         * \brief Create a new placed item.
         */
        AIArtHandle CreatePlacedItem(const ai::FilePath& pdf_path);

        /**
         * \brief Set the placement of the placed item, acording to the option in property.
         */
        void SetPlacement(const AIArtHandle& placed_item, const L2A::Property& item_property);

        /**
         * \brief Set the placement of the placed item, acording to the a1 enums.
         */
        void SetPlacement(const AIArtHandle& placed_item, const PlaceMethod& method, const PlaceAlignment& alignment,
            const bool& clip);

        /**
         * \brief Get the ai enums placement of the placed item.
         */
        std::tuple<PlaceMethod, PlaceAlignment, AIBoolean> GetPlacement(const AIArtHandle& placed_item);

        /**
         * \brief Get the filepath of the linked file in a placed item.
         */
        ai::FilePath GetPlacedItemPath(const AIArtHandle& placed_item);

        /**
         * \brief Set the filepath of the linked file in a placed item.
         */
        void SetPlacedItemPath(AIArtHandle& placed_item, const ai::FilePath& path);

        /**
         * \brief Relink a placed item with a file.
         *
         * The difference to SetPlacedItemPath is that here the boundary box is reset.
         * The note and name have to be updated after this.
         *
         * @param placed_item (in/out) placed item to be linked. The new linked item is assigned to this variable.
         * @param path (in) Path to the file.
         */
        void RelinkPlacedItem(AIArtHandle& placed_item, const ai::FilePath& path);

        /**
         * \brief Set the note for an ai item.
         */
        void SetNote(const AIArtHandle& item, const ai::UnicodeString& note);

        /**
         * \brief Get the note for an ai item.
         */
        ai::UnicodeString GetNote(const AIArtHandle& item);

        /**
         * \brief Set the name for an ai item.
         */
        void SetName(const AIArtHandle& item, const ai::UnicodeString& name);

        /**
         * \brief Set the name for an ai item.
         */
        ai::UnicodeString GetName(const AIArtHandle& item);

        /**
         * \brief Get the art bounds of an art handle.
         */
        AIRealRect GetArtBounds(const AIArtHandle& item);

        /**
         * \brief Get the placement matrix of an placed item.
         */
        AIRealMatrix GetPlacedMatrix(const AIArtHandle& placed_item);

        /**
         * \brief Get the boundary box of a placed item.
         */
        AIRealRect GetPlacedBoundingBox(const AIArtHandle& placed_item);

        /**
         * \brief Transform an art item with the given matrix.
         */
        void TransformArt(const AIArtHandle& placed_item, AIRealMatrix& artMatrix);

        /**
         * \brief Set the placed matrix for a placed item.
         */
        void SetPlacedMatrix(const AIArtHandle& placed_item, AIRealMatrix& placed_matrix);

        /**
         * \brief Convert the alignment flag to the 2d positional factors.
         */
        void AlignmentToFac(const PlaceAlignment& alignment, AIReal (&pos_fac)[2]);

        /**
         * \brief Check if the given item is a l2a item.
         */
        bool IsL2AItem(const AIArtHandle& item);

        /**
         * \brief Check if isolation mode is active in the document.
         */
        bool IsIsolationMode();

        /**
         * \brief Exit the current isolation mode.
         */
        void ExitIsolationMode();

        /**
         * \brief Get art handles in the document.
         * @param items (out) Vector that will be cleared and filled up with the found items.
         * @param selected (in) Which selection state should be searched.
         * @param type (in) Type of the items to search.
         */
        void GetItems(std::vector<AIArtHandle>& items, SelectionState selected, ai::int16 type = kAnyArt);

        /** Gets all placed items that are L2A items in the current document.
         * @param l2a_items (out) vector that returns the art handles that match the description.
         * @param selected (in) Type of selected items to search.
         */
        void GetDocumentItems(std::vector<AIArtHandle>& l2a_items, SelectionState selected);

        /**
         * \brief Get the art item that is in single isolation mode.
         * @return False if no single art item is in isolation mode.
         */
        bool GetSingleIsolationItem(AIArtHandle& item);

        /**
         * \brief Check if the active document is a cloud document.
         */
        bool IsActiveDocumentCloudDocument();

        /**
         * \brief Activate undo tracking.
         */
        void UndoActivate();

        /**
         * \brief Deactivate undo tracking.
         */
        void UndoDeactivate();

        /**
         * \brief Set undo state.
         */
        void UndoSetActive(bool silent);

        /**
         * \brief Set undo text
         */
        void SetUndoText(const ai::UnicodeString& undo_text, const ai::UnicodeString& redo_text);

        /**
         * \brief Get the number of active documents.
         */
        unsigned int GetDocumentCount();

        /**
         * \brief Convert a point in the document to a point on the screen.
         * @param artwork_point Point in the document.
         * @return Point on the screen
         */
        AIPoint ArtworkPointToViewPoint(const AIRealPoint& artwork_point);

        /**
         * \brief Converts an AIRealRect in artwork coordinates to an AIRect in view coordinates.
         * @param artworkBounds IN the rectangle bounds in artwork coordinates.
         * @return the rectangle bounds in view coordinates.
         */
        AIRect ArtworkBoundsToViewBounds(const AIRealRect& artwork_bounds);

        /**
         * \brief Save a copy of the active document to a pdf.
         */
        void SaveToPDF();

        /**
         * \brief Save a copy of the active document to an Illustrator file.
         */
        void SaveAs(const ai::FilePath& document_path);

        /**
         * \brief Rotate the currently active Illustrator item(s). With this rotate, the boundary box is also rotated
         * (opposed to the internal rotation functions).
         * @param anlge Angle in degrees to rotate.
         */
        void Rotate(const AIReal angle);

        /**
         * \brief Reset the bounding box of the currently active Illustrator item(s).
         * @param item AI item to reset the boundary box on.
         */
        void ResetBoundingBox();

        /**
         * \brief Move the currently acitve Illustrator item(s).
         * @param delta_x Offset in x direction (in points)
         * @param delta_y Offset in y direction (in points)
         * @param copy If the item chould be moved or copied and then moved.
         */
        void Move(const AIReal delta_x, const AIReal delta_y, bool copy = false);

        /**
         * \brief Scale the currently acitve Illustrator item(s).
         * @param scale_x Horizontal scale (unitless).
         * @param scale_y Vertical scale (unitless).
         * @param copy If the item chould be copied.
         */
        void Scale(const AIReal scale_x, const AIReal scale_y, bool copy = false);

        /**
         * \brief Select all items in the items vector.
         */
        void SelectItems(std::vector<AIArtHandle>& items);

        /**
         * \brief Draw a path and return the created art handle.
         */
        void DrawPath(const std::vector<AIPathSegment>& segments, AIPathStyle style, bool closed);

        /**
         * \brief Get the points on a path art item.
         * @params path_item (in) Path item to get the points from.
         * @params points (out) Vector with the path points.
         * @params append_to_vector (in) If the points should be appended to vector, of if vector shoyuld be cleared
         * first.
         */
        void GetPathPoints(AIArtHandle& path_item, std::vector<AIRealPoint>& points, bool append_to_vector = false);

        /**
         * \brief Get the parent item of the art item.
         * @params art_item (in) Art item.
         * @params parent (out) Parent art item.
         * @return True if parent exists.
         */
        bool GetArtParent(const AIArtHandle& art_item, AIArtHandle& parent);

        /**
         * \brief Get all parents of an art item.
         * @params art_item (in) Art item.
         * @params parents (out) Parents of the art item. The first item in the vector will be top parent, i.e. the main
         * layer.
         */
        void GetArtParents(const AIArtHandle& art_item, std::vector<AIArtHandle>& parents);

        /**
         * \brief Check if an item is locked and or hidden in AI, including the parent locked / hidden settings.
         * @params art_item (in) Art item.
         * @params is_hidden (out) If the item or any of the parents are hidden.
         * @params is_locked (out) If the item or any of the parents are locked.
         * layer.
         */
        void GetIsHiddenLocked(const AIArtHandle& art_item, bool& is_hidden, bool& is_locked);

        /**
         * \brief Check if an item (internally that is a pointer) is valid.
         * @params art_item (in) Art item.
         */
        bool IsValidArt(const AIArtHandle& art_item);

        /**
         * \brief Check if the current insertion point is locked and or hidden, i.e. no item can be created.
         */
        bool GetLockedInsertionPoint();

        /**
         * \brief Ask the user a Yes or no question.
         */
        bool YesNoAlert(const ai::UnicodeString& message);

        /**
         * \brief Get string input from the user.
         *
         * Should oly be used for debug purposes. With the current implementation, the user can not cancel the dialog
         */
        ai::UnicodeString GetInputFromUser();

        /**
         * \brief Provides the MessageAlert functionality, which can be turned off for testing purposes
         *  TODO: Allow to optionally activate the testing stuff, so we can display the error message on failed tests
         *  TODO: Apply this function to all occurrences of message alert in L2A
         */
        void MessageAlert(const ai::UnicodeString& message_string);

        /**
         * \brief Provides the WarningAlert functionality, which can be turned off for testing purposes
         */
        void WarningAlert(const ai::UnicodeString& warning_string);

    }  // namespace AI
}  // namespace L2A

#endif
