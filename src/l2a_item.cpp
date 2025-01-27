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
 * \brief Errors for l2a applications.
 */


#include "IllustratorSDK.h"

#include "l2a_item.h"

#include "l2a_ai_functions.h"
#include "l2a_constants.h"
#include "l2a_error.h"
#include "l2a_file_system.h"
#include "l2a_global.h"
#include "l2a_latex.h"
#include "l2a_math.h"
#include "l2a_names.h"
#include "l2a_parameter_list.h"
#include "l2a_plugin.h"
#include "l2a_string_functions.h"
#include "l2a_suites.h"
#include "l2a_ui_manager.h"
#include "l2a_utils.h"


/**
 *
 */
L2A::Item::Item(const AIRealPoint& position, const L2A::Property& property, const ai::FilePath& created_pdf_file)
{
    // TODO: Maybe move this to a factory function that can give better error return values

    // Set the property 1for this item
    property_ = property;

    // Store the pdf data in the property
    property_.SetPDFFile(created_pdf_file);

    // Save the pdf in the pdf folder
    const auto pdf_file = GetPDFPath();
    SaveEncodedPDFFile(pdf_file);

    // Create the placed item
    placed_item_ = L2A::AI::CreatePlacedItem(pdf_file);
    L2A::AI::SetPlacement(placed_item_, property_);

    // Set the name and tag for the item
    SetNoteAndName();

    // Move the file to the cursor position
    MoveItem(position);

    // Everything was successful, we write the input from this item to the application directory, so we can use it with
    // the next one.
    property_.WriteLastInput();
}

/**
 *
 */
L2A::Item::Item(const AIArtHandle& placed_item_handle)
{
    // Set placed item object.
    placed_item_ = placed_item_handle;

    // Get the data from the art item.
    property_ = L2A::Property();
    property_.SetFromString(L2A::AI::GetNote(placed_item_));

    // Check that the placed options are correctly in sync with AI and that the stretch behavior is set to fit to
    // boundary box
    {
        // Placement of the LaTeX2AI item
        const auto [method_l2a, clip_l2a] = L2A::UTIL::KeyToValue(
            PlacedArtMethodEnums(), PlacedArtMethodEnumsAI(), L2A::PlacedArtMethod::fill_to_boundary_box);
        PlaceAlignment alignment_l2a = property_.GetAIAlignment();

        // Actual placement settings of the placed art in Illustrator
        const auto [method_placed_item, alignment_placed_item, clip_placed_item] = L2A::AI::GetPlacement(placed_item_);

        if (method_placed_item != method_l2a || alignment_placed_item != alignment_l2a || clip_placed_item != clip_l2a)
        {
            if (alignment_placed_item == alignment_l2a && property_.GetVersion() < L2A::UTIL::ParseVersion("1.0.0"))
            {
                // Starting from v1.0.0 we only support items that are stretched to the boundary box for simplicity
                // reasons. No warning is needed here.
            }
            else
            {
                // The options do not match, apply the ones from the LaTeX2AI options.
                sAIUser->MessageAlert(ai::UnicodeString(
                    "The Illustrator placement values for a LaTeX2AI item do not match! The ones from the LaTeX2AI "
                    "settings are applied. This can happen if the placement values are changed manually via "
                    "Illustrator."));
            }

            // Set the correct placement options
            L2A::AI::SetPlacement(placed_item_, property_);
        }
    }
}

/**
 *
 */
L2A::ItemChangeResult L2A::Item::Change(const ai::UnicodeString& form_return_value, L2A::Property& new_property)
{
    L2A::AI::SetUndoText(
        ai::UnicodeString("Undo Change LaTeX2AI Item"), ai::UnicodeString("Redo Change LaTeX2AI Item"));

    L2A::PropertyCompare diff;

    if (form_return_value == "ok")
    {
        // Compare the current input and the new one
        diff = GetProperty().Compare(new_property);
    }
    else if (form_return_value == "redo_boundary")
    {
        RedoBoundary();
        return ItemChangeResult{ItemChangeResult::Result::ok};
    }
    else if (form_return_value == "redo_latex")
    {
        diff.changed_latex = true;
        // We have to set the property here since the redo latex button will redo the existing item, not the text that
        // might be changed in the UI. We raise a warning for that case in the UI.
        // TODO: maybe return the correct data directly?
        // TODO: copy here without copying the pdf contents
        new_property = GetProperty();
    }
    else
    {
        l2a_error("Got unexpected form_return_value: " + form_return_value);
    }

    if (diff.changed_latex)
    {
        auto [latex_creation_result, pdf_file] = L2A::LATEX::CreateLatexItem(new_property);
        if (latex_creation_result.result_ == L2A::LATEX::LatexCreationResult::Result::ok)
        {
            // TODO: this works, but it is very strange what we copy around here, this should be improved
            // PDF could be created, now store the pdf file in the placed item
            new_property.SetPDFFile(pdf_file);
            GetPropertyMutable() = new_property;
            pdf_file = GetPDFPath();
            SaveEncodedPDFFile(pdf_file);

            // Relink the placed item with the new pdf file
            L2A::AI::RelinkPlacedItem(GetPlacedItemMutable(), pdf_file);

            // Redo the boundary
            RedoBoundary();
        }
        else if (latex_creation_result.result_ == L2A::LATEX::LatexCreationResult::Result::error_tex_code)
        {
            // The LaTeX call worked, but the LaTeX code resulted in errors -> ask the user to fix the code
            return ItemChangeResult{ItemChangeResult::Result::latex_error, latex_creation_result};
        }
        else
        {
            // An error occurred that is not caused by bad LaTeX code. Likely reasons can be a bad file path to TeX or
            // ghostscript. In this case we cancel the item creation.
            return ItemChangeResult{ItemChangeResult::Result::cancel};
        }
    }

    // If something changed add the information to the placed item note
    if (diff.Changed())
    {
        GetPropertyMutable() = new_property;
        SetNoteAndName();

        if (diff.changed_align)
        {
            L2A::AI::SetPlacement(GetPlacedItemMutable(), GetProperty());
        }
    }

    // Everything worked fine
    return ItemChangeResult{ItemChangeResult::Result::ok};
}

/**
 *
 */
void L2A::Item::RedoBoundary()
{
    L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("L2A::Item::RedoBoundary Line ") + L2A::UTIL::IntegerToString(__LINE__));

    // If object is not stretched and not diamond -> do nothing.
    if (!IsStretched() && !IsDiamond()) return;

    // Get the position of the reference point.
    AIRealPoint old_position = GetPosition();

    // Get the angle.
    L2A::GlobalMutable().logger_.push_back(ai::UnicodeString("L2A::Item::RedoBounary ")+ L2A::UTIL::IntegerToString(__LINE__));
    AIReal angle = GetAngle();

    // Rotate the object back to the initial position.
    AIRealMatrix artMatrix;
    sAIRealMath->AIRealMatrixSetRotate(&artMatrix, (ASReal)(-angle));
    L2A::AI::TransformArt(placed_item_, artMatrix);

    // Set the transformation matrix to identity matrix.
    artMatrix.a = 1.;
    artMatrix.b = 0.;
    artMatrix.c = 0.;
    artMatrix.d = -1.;
    L2A::AI::SetPlacedMatrix(placed_item_, artMatrix);

    // Rotate back
    sAIRealMath->AIRealMatrixSetRotate(&artMatrix, (ASReal)(angle));
    L2A::AI::TransformArt(placed_item_, artMatrix);

    // Relink the file so that the boundary box is reset.
    L2A::AI::RelinkPlacedItem(placed_item_, L2A::AI::GetPlacedItemPath(placed_item_));
    SetNoteAndName();

    // Move back to original position.
    MoveItem(old_position);
}

/**
 *
 */
void L2A::Item::SetNoteAndName() const
{
    L2A::AI::SetNote(placed_item_, property_.ToString(true));
    L2A::AI::SetName(placed_item_, ai::UnicodeString(L2A::NAMES::ai_item_name_));
}

/**
 *
 */
ai::FilePath L2A::Item::GetPDFPath() const
{
    ai::UnicodeString pdf_file_hash = property_.GetPDFFileHash();
    if (pdf_file_hash.empty()) l2a_error("File hash should not be empty.");
    ai::FilePath pdf_path = L2A::UTIL::GetPdfFileDirectory();
    ai::UnicodeString document_name = L2A::UTIL::GetDocumentName();
    pdf_path.AddComponent(document_name + L2A::NAMES::pdf_item_post_fix_ + pdf_file_hash + ".pdf");
    return pdf_path;
}

/**
 *
 */
void L2A::Item::SaveEncodedPDFFile(const ai::FilePath& pdf_path) const
{
    // Make sure the directory exists.
    if (!L2A::UTIL::IsDirectory(pdf_path.GetParent())) L2A::UTIL::CreateDirectoryL2A(pdf_path.GetParent());

    const ai::UnicodeString& pdf_contents = property_.GetPDFFileContents();
    if (!pdf_contents.empty())
        L2A::UTIL::decode_file_base64(pdf_path, pdf_contents);
    else
        l2a_error("Could not save the encoded pdf file, got empty encoded data.");
}

/**
 *
 */
ai::UnicodeString L2A::Item::GetAIName() const { return L2A::AI::GetName(placed_item_); }

/**
 *
 */
void L2A::Item::MoveItem(const AIRealPoint& position)
{
    // Get the current position of the placed art item.
    AIRealPoint old_position = GetPosition();

    // For some objects that are far out of the artwork, multiple translations can be necessary to reach the desired
    // position transform the object until the desired position is reached.
    AIReal position_error = 1.;
    int counter = 0;
    const int n_max = 3;

    while (position_error > L2A::CONSTANTS::eps_pos_)
    {
        if (n_max == counter)
            l2a_error(
                "L2A::Item::MoveItem Desired position not reached in " + L2A::UTIL::IntegerToString(n_max) + " tries!");

        // Transform the art item to the desired position.
        AIRealMatrix artMatrix;
        sAIRealMath->AIRealMatrixSetTranslate(&artMatrix, position.h - old_position.h, position.v - old_position.v);
        L2A::AI::TransformArt(placed_item_, artMatrix);

        // Check error in new position.
        old_position = GetPosition();
        position_error = L2A::UTIL::MATH::GetDistance(position, old_position);
        counter++;
    }
}

/**
 *
 */
AIRealPoint L2A::Item::GetPosition() const
{
    std::vector<PlaceAlignment> alignment_vector;
    alignment_vector.clear();
    alignment_vector.push_back(property_.GetAIAlignment());
    return GetPosition(alignment_vector).at(0);
}

/**
 *
 */
std::vector<AIRealPoint> L2A::Item::GetPosition(const std::vector<PlaceAlignment>& placements) const
{
    // Vector of points to return.
    std::vector<AIRealPoint> positions;

    // Variables for the placement factor and the points on the item.
    AIReal pos_fac[2];
    AIRealPoint position;

    // Boundary coordinates for item.
    AIRealRect bounds = L2A::AI::GetArtBounds(placed_item_);

        L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("L2A::Item::GetPosition Line ") + L2A::UTIL::IntegerToString(__LINE__));
    if ((!IsRotated()) && (!IsDiamond()))
    {
        // Item is rectangle that is not rotated. This should be the default case.

        // Get the coordinates of the placement point.
        for (const auto& alignment : placements)
        {
            L2A::AI::AlignmentToFac(alignment, pos_fac);
            position.h = (ASReal)(bounds.left + pos_fac[0] * (bounds.right - bounds.left));
            position.v = (ASReal)(bounds.bottom + pos_fac[1] * (bounds.top - bounds.bottom));
            positions.push_back(position);
        }
    }
    else
    {
        // Angles of the basis vectors
        L2A::GlobalMutable().logger_.push_back(
            ai::UnicodeString("L2A::Item::GetPosition ") + L2A::UTIL::IntegerToString(__LINE__));
        AIReal angle_1 = GetAngle(0);
        AIReal angle_2 = GetAngle(1);

        // Scale factor of the basis vectors.
        AIReal scale_1 = GetStretch(0);
        AIReal scale_2 = GetStretch(1);

        // Dimensions of the pdf file.
        AIRealRect image_box = L2A::AI::GetPlacedBoundingBox(placed_item_);
        AIReal pdf_height = image_box.top - image_box.bottom;
        AIReal pdf_width = image_box.right - image_box.left;

        // Vectors to each corner of the item.
        AIReal vec0[2] = {0., 0.};
        AIReal vec1[2] = {scale_1 * pdf_width * cos(angle_1), scale_1 * pdf_width * sin(angle_1)};
        AIReal vec2[2] = {scale_2 * pdf_height * cos(angle_2), scale_2 * pdf_height * sin(angle_2)};
        AIReal vec3[2] = {vec1[0] + vec2[0], vec1[1] + vec2[1]};

        // Get the minimum distance for both directions.
        AIReal diff_x = min(min(min(vec0[0], vec1[0]), vec2[0]), vec3[0]);
        AIReal diff_y = min(min(min(vec0[1], vec1[1]), vec2[1]), vec3[1]);

        // Position of the bottom left node.
        AIReal pos_node[2] = {bounds.left - diff_x, bounds.bottom - diff_y};

        // Get the coordinates of the placement point.
        for (const auto& alignment : placements)
        {
            L2A::AI::AlignmentToFac(alignment, pos_fac);
            position.h = (ASReal)(pos_node[0] + pos_fac[0] * vec1[0] + pos_fac[1] * vec2[0]);
            position.v = (ASReal)(pos_node[1] + pos_fac[0] * vec1[1] + pos_fac[1] * vec2[1]);
            positions.push_back(position);
        }
    }

    return positions;
}

/**
 *
 */
void L2A::Item::Draw(AIAnnotatorMessage* message, const std::map<PlaceAlignment, AIRealPoint>& item_boundaries) const
{
    auto is_valid = sAIArt->ValidArt(this->placed_item_, true);
    
    L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("L2A::Item::Draw Line ") + L2A::UTIL::IntegerToString(__LINE__));
    
    L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("  Valid Art: ") + L2A::UTIL::IntegerToString(is_valid));

    // Get the color for this item.
    AIRGBColor item_color;
    if (IsDiamond())
        item_color = L2A::CONSTANTS::color_diamond_;
    else if (IsStretched())
        item_color = L2A::CONSTANTS::color_scaled_;
    else
        item_color = L2A::CONSTANTS::color_ok_;

    // Get the hidden and locked status of the item.
    bool is_hidden;
    bool is_locked;
    L2A::AI::GetIsHiddenLocked(placed_item_, is_hidden, is_locked);
    if (is_hidden) return;
    if (is_locked)
    {
        item_color.red = ai::uint16(0.5 * item_color.red);
        item_color.green = ai::uint16(0.5 * item_color.green);
        item_color.blue = ai::uint16(0.5 * item_color.blue);
    }

    // Set drawing options.
    sAIAnnotatorDrawer->SetLineWidth(message->drawer, (AIReal)(L2A::CONSTANTS::line_width_));
    sAIAnnotatorDrawer->SetColor(message->drawer, item_color);
    sAIAnnotatorDrawer->SetLineDashed(message->drawer, false);


    // Get the coordinates of all Array with the relevant view positions.
    std::vector<AIPoint> polygon_points_view;
    for (auto const& placement : {kMidLeft, kTopLeft, kTopRight, kBotRight, kBotLeft, kMidLeft})
        polygon_points_view.push_back(L2A::AI::ArtworkPointToViewPoint(item_boundaries.at(placement)));
    std::vector<AIPoint> baseline_points_view;
    for (auto const& placement : {kMidLeft, kMidRight})
        baseline_points_view.push_back(L2A::AI::ArtworkPointToViewPoint(item_boundaries.at(placement)));

    // Draw the boundary.
    AIErr error = sAIAnnotatorDrawer->DrawPolygon(
        message->drawer, polygon_points_view.data(), (ai::uint32)polygon_points_view.size(), false);
    l2a_check_ai_error(error);

    // Draw the placement point.
    AIPoint placement_point = L2A::AI::ArtworkPointToViewPoint(item_boundaries.at(property_.GetAIAlignment()));
    AIRect centre;
    centre.left = placement_point.h - L2A::CONSTANTS::radius_;
    centre.right = placement_point.h + L2A::CONSTANTS::radius_;
    centre.bottom = placement_point.v - L2A::CONSTANTS::radius_;
    centre.top = placement_point.v + L2A::CONSTANTS::radius_;
    error = sAIAnnotatorDrawer->DrawEllipse(message->drawer, centre, true);
    l2a_check_ai_error(error);

    if (property_.IsBaseline())
    {
        // Dash data for dashed line to display baseline items.
#if kPluginInterfaceVersion >= 0x17000001
        std::vector<AIFloat> dash_data_ = {20, 7};
#else
        std::vector<AIReal> dash_data_ = {20, 7};
#endif

        // Draw the base line of a baseline item.
        error = sAIAnnotatorDrawer->SetLineDashedEx(message->drawer, &dash_data_[0], (ai::int32)dash_data_.size());
        error = sAIAnnotatorDrawer->DrawLine(message->drawer, baseline_points_view[0], baseline_points_view[1]);
        l2a_check_ai_error(error);
    }
}

/**
 *
 */
AIReal L2A::Item::GetAngle(unsigned short director) const
{
    // Get the values from the matrix.
    L2A::GlobalMutable().logger_.push_back(ai::UnicodeString("L2A::Item::GetAngle"));
    L2A::GlobalMutable().logger_.push_back(ai::UnicodeString("  PDF Path:\n     ") + this->GetPDFPath().GetFullPath());
    L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("  Exists: ") + L2A::UTIL::IntegerToString(L2A::UTIL::IsFile(this->GetPDFPath())));
    AIRealMatrix artMatrix = L2A::AI::GetPlacedMatrix(placed_item_);

    if (director == 0)
        return -atan2(artMatrix.b, artMatrix.a);
    else
        return atan2(-artMatrix.d, artMatrix.c);
}

/**
 *
 */
bool L2A::Item::IsRotated() const
{
    // check if item is rotated
    L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("L2A::Item::IsRotated ") + L2A::UTIL::IntegerToString(__LINE__));
    if (abs(GetAngle()) < L2A::CONSTANTS::eps_angle_)
        return false;
    else
        return true;
}

/**
 *
 */
bool L2A::Item::IsDiamond() const
{
    // Check if the angle between the two directors is pi/2
    L2A::GlobalMutable().logger_.push_back(
        ai::UnicodeString("L2A::Item::IsDiamond ") + L2A::UTIL::IntegerToString(__LINE__));
    AIReal angle_1 = GetAngle(0);
    AIReal angle_2 = GetAngle(1);
    // Use the strech tollerance here, because not the angles are compared, but their cosines.
    if (abs(cos(angle_2 - angle_1)) < L2A::CONSTANTS::eps_strech_)
        return false;
    else
        return true;
}

/**
 *
 */
AIReal L2A::Item::GetStretch(unsigned short director) const
{
    // Get the values from the matrix.
    L2A::GlobalMutable().logger_.push_back(ai::UnicodeString("L2A::Item::GetStretch"));

    L2A::GlobalMutable().logger_.push_back(ai::UnicodeString("  PDF Path:\n     ") + this->GetPDFPath().GetFullPath());
    L2A::GlobalMutable().logger_.push_back(ai::UnicodeString("  Exists: ") + L2A::UTIL::IntegerToString(L2A::UTIL::IsFile(this->GetPDFPath())));

    AIRealMatrix artMatrix = L2A::AI::GetPlacedMatrix(placed_item_);

    if (director == 0)
        return sqrt(artMatrix.b * artMatrix.b + artMatrix.a * artMatrix.a);
    else
        return sqrt(artMatrix.c * artMatrix.c + artMatrix.d * artMatrix.d);
}

/**
 *
 */
bool L2A::Item::IsStretched() const
{
    // Check if item is streched, both strech factors must be smaller than eps.
    if ((abs(1. - GetStretch(0)) < L2A::CONSTANTS::eps_strech_) &&
        (abs(1. - GetStretch(1)) < L2A::CONSTANTS::eps_strech_))
        return false;
    else
        return true;
}

/**
 *
 */
void L2A::RedoItems(std::vector<AIArtHandle>& redo_items, const RedoItemsOption& redo_option)
{
    L2A::AI::SetUndoText(ai::UnicodeString("Undo Redo LaTeX2AI Items"), ai::UnicodeString("Redo LaTeX2AI Items"));

    // Check if something needs to be done
    if (redo_items.size() == 0) return;

    // Create an Item for all placed items that need to be redone
    unsigned int locked_counter = 0;
    std::vector<L2A::Item> l2a_items;
    for (const auto& placed_item : redo_items)
    {
        bool is_hidden;
        bool is_locked;
        L2A::AI::GetIsHiddenLocked(placed_item, is_hidden, is_locked);
        if (is_hidden || is_locked)
            locked_counter++;
        else
            l2a_items.push_back(L2A::Item(placed_item));
    }

    // Aller the user that locked and or hidden items were selected.
    if (locked_counter > 0)
    {
        ai::UnicodeString message_text("The LaTeX2AI Redo function got ");
        message_text += L2A::UTIL::IntegerToString(locked_counter);
        message_text += " hidden / locked items. They will be skipped.";
        sAIUser->MessageAlert(message_text);
    }

    if (redo_option == RedoItemsOption::latex)
    {
        if (!RedoLaTeXItems(l2a_items)) return;
    }

    // Redo the boundaries of all items (this has to be done for both cases of redo_option
    for (auto& item : l2a_items) item.RedoBoundary();
}

/**
 *
 */
bool L2A::RedoLaTeXItems(std::vector<L2A::Item>& l2a_items)
{
    // Loop over every element and get the property
    std::vector<L2A::Property> properties;
    for (const auto& item : l2a_items)
    {
        // TODO: dont copy the pdf contents here
        properties.push_back(item.GetProperty());
    }

    // Create the pdf file for each item
    auto [latex_creation_result, pdf_files] = L2A::LATEX::CreateLatexItems(properties);
    if (latex_creation_result.result_ != L2A::LATEX::LatexCreationResult::Result::ok)
    {
        L2A::GlobalPluginMutable().GetUiManager().GetDebugForm().OpenDebugForm(
            L2A::UI::Debug::Action::redo_items, latex_creation_result);
        return false;
    }

    // Create the PDFs for the items and store them in the placed items. We dont reset the boundary box here. This is
    // done in the redo function, we leave it out here, since one might want to use this function without resetting the
    // bounding box.
    for (unsigned int i = 0; i < l2a_items.size(); i++)
    {
        // Get the PDF path.
        auto& l2a_item = l2a_items[i];
        l2a_item.GetPropertyMutable().SetPDFFile(pdf_files[i]);
        ai::FilePath new_path = l2a_item.GetPDFPath();
        l2a_item.SaveEncodedPDFFile(new_path);
        L2A::AI::RelinkPlacedItem(l2a_item.GetPlacedItemMutable(), new_path);
        l2a_item.SetNoteAndName();
    }

    return true;
}

/**
 *
 */
void L2A::CheckItemDataStructure()
{
    // We need a valid document path for this function to work.
    if (!L2A::UTIL::IsFile(L2A::UTIL::GetDocumentPath(false))) return;

    // Get all LaTeX2AI placed items in this document.
    std::vector<AIArtHandle> items_all;
    L2A::AI::GetDocumentItems(items_all, L2A::AI::SelectionState::all);

    // Loop over each item and check if the pdf file is encoded and stored within the item.
    std::vector<L2A::Item> working_items;
    std::vector<L2A::Item> redo_items;
    for (auto& item : items_all)
    {
        const ai::FilePath old_path = L2A::AI::GetPlacedItemPath(item);

        L2A::Item l2a_item(item);
        if (!l2a_item.GetProperty().GetPDFFileHash().empty())
        {
            working_items.push_back(l2a_item);
        }
        else if (L2A::UTIL::IsFile(old_path))
        {
            // No pdf data is stored in the item and the pdf file exists -> Save the data to the item.
            l2a_item.GetPropertyMutable().SetPDFFile(old_path);
            l2a_item.SetNoteAndName();
            working_items.push_back(l2a_item);
        }
        else
        {
            // The LaTeX label has to be redone.
            redo_items.push_back(l2a_item);
        }
    }

    if (redo_items.size() > 0)
    {
        // Ask the user if the missing items should be redone.
        ai::UnicodeString message_string("There are ");
        message_string += L2A::UTIL::IntegerToString((int)redo_items.size());
        message_string +=
            " LaTeX2AI item(s) in the document that have to be redone due to missing PDF links.\nRedo now?";
        if (L2A::AI::YesNoAlert(message_string))
        {
            RedoLaTeXItems(redo_items);

            // Add the items to the working_items vector.
            for (auto& item : redo_items) working_items.push_back(item);
        }
    }

    // Make sure the pdf folder exists.
    const ai::FilePath pdf_file_directory = L2A::UTIL::GetPdfFileDirectory();
    if (working_items.size() > 0) L2A::UTIL::CreateDirectoryL2A(pdf_file_directory);

    // Loop over each LaTeX2AI item and check if it is stored correctly.
    std::vector<ai::FilePath> used_pdf_files;
    for (auto& item : working_items)
    {
        const ai::FilePath new_pdf_path = item.GetPDFPath();
        const ai::FilePath old_pdf_path = L2A::AI::GetPlacedItemPath(item.GetPlacedItem());
        if (!(L2A::UTIL::IsEqualFile(new_pdf_path, old_pdf_path) && L2A::UTIL::IsFile(new_pdf_path)))
        {
            // Store the pdf in the correct path and relink it.
            item.SaveEncodedPDFFile(new_pdf_path);
            L2A::AI::SetPlacedItemPath(item.GetPlacedItemMutable(), new_pdf_path);
        }
        used_pdf_files.push_back(new_pdf_path);
    }

    // Cleanup pdf links directory.
    {
        // Get all pdf items.
        ai::UnicodeString pattern = ai::UnicodeString(".*") + L2A::NAMES::pdf_item_post_fix_ + ".*\\.pdf$";
        std::vector<ai::FilePath> pdf_item_files = L2A::UTIL::FindFilesInFolder(pdf_file_directory, pattern);

        // Get all documents parallel to the current document.
        pattern = ai::UnicodeString(".*\\.ai$");
        std::vector<ai::FilePath> ai_document_files =
            L2A::UTIL::FindFilesInFolder(L2A::UTIL::GetDocumentPath().GetParent(), pattern);

        // Loop over each pdf label and check if it should be deleted.
        for (const auto& pdf_path : pdf_item_files)
        {
            bool delete_this_file = true;

            // Check if the pdf if part of this document.
            for (const auto& used_file : used_pdf_files)
            {
                if (pdf_path == used_file) delete_this_file = false;
            }

            // Check if the pdf fits to another Illustrator document by its name.
            const ai::UnicodeString pdf_name = pdf_path.GetFileName();
            for (const auto& ai_file : ai_document_files)
            {
                // Do not check the name of this document, as the individual items are already checked above.
                if (L2A::UTIL::GetDocumentPath() == ai_file) continue;

                ai::UnicodeString ai_name = ai_file.GetFileNameNoExt();
                if (L2A::UTIL::StartsWith(pdf_name, ai_name + L2A::NAMES::pdf_item_post_fix_, true))
                    delete_this_file = false;
            }

            // If the file is not used by this document or shares the name with an existing Illustrator document, delete
            // it.
            if (delete_this_file) L2A::UTIL::RemoveFile(pdf_path);
        }
    }
}
