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
 * \brief Form for creating / editing an item
 */

#include "IllustratorSDK.h"

#include "l2a_ui_item.h"

#include "l2a_ai_functions.h"
#include "l2a_global.h"
#include "l2a_latex.h"
#include "l2a_parameter_list.h"
#include "l2a_plugin.h"
#include "l2a_string_functions.h"


/**
 * \brief Set the names for item forms
 */
const std::string L2A::UI::Item::FORM_NAME = "LaTeX2AI create/edit item";
const std::string L2A::UI::Item::FORM_ID = "com.adobe.illustrator.latex2aiui.dialog_item";
const std::string L2A::UI::Item::EVENT_TYPE_BASE = "com.adobe.csxs.events.latex2ai.item";
const std::string L2A::UI::Item::EVENT_TYPE_READY = L2A::UI::Item::EVENT_TYPE_BASE + ".ready";
const std::string L2A::UI::Item::EVENT_TYPE_OK = L2A::UI::Item::EVENT_TYPE_BASE + ".ok";
const std::string L2A::UI::Item::EVENT_TYPE_UPDATE = L2A::UI::Item::EVENT_TYPE_BASE + ".update";
const std::string L2A::UI::Item::EVENT_TYPE_SET_CLOSE_ON_FOCUS = L2A::UI::Item::EVENT_TYPE_BASE + ".set_close_on_focus";


/**
 *
 */
L2A::UI::Item::Item() : FormBase(FORM_NAME, FORM_ID.c_str(), EVENT_TYPE_BASE)
{
    // If we don't do this this way, we get a compiler error
    std::vector<EventListenerData> event_listener_data = {
        {EVENT_TYPE_READY, CallbackHandler<Item, &Item::CallbackFormReady>()},  //
        {EVENT_TYPE_OK, CallbackHandler<Item, &Item::CallbackOk>()}             //
    };
    event_listener_data_ = std::move(event_listener_data);
}

/**
 *
 */
void L2A::UI::Item::ResetFormData()
{
    action_type_ = ActionType::none;
    change_item_ = nullptr;
}

/**
 *
 */
void L2A::UI::Item::OpenCreateItemForm(const AIRealPoint& position)
{
    // TODO:Check if document is saved

    action_type_ = ActionType::create_item;
    new_item_insertion_point_ = position;

    // Set the property to the last successfull newly created item value
    property_.SetFromLastInput();

    LoadForm();
}

/**
 *
 */
void L2A::UI::Item::OpenEditItemForm(const AIArtHandle& art_handle)
{
    // TODO:Check if document is saved

    action_type_ = ActionType::edit_item;
    change_item_ = std::make_unique<L2A::Item>(art_handle);
    property_ = change_item_->GetProperty();

    LoadForm();
}

/**
 *
 */
void L2A::UI::Item::CallbackFormReady(const csxs::event::Event* const eventParam) { SendData(); }

/**
 *
 */
void L2A::UI::Item::CallbackOk(const csxs::event::Event* const eventParam)
{
    // We need to activate the app context here, because otherwise functions like the GetDocumentName will not work
    auto app_context = L2A::GlobalPluginAppContext();

    // Convert the return data to a parameter list
    L2A::UTIL::ParameterList form_return_data(L2A::UTIL::StringStdToAi(eventParam->data));

    if (action_type_ == ActionType::create_item)
    {
        CreateNewItem(*form_return_data.GetSubList(ai::UnicodeString("l2a_item")));
    }
    else if (action_type_ == ActionType::edit_item)
    {
        EditItem(form_return_data.GetStringOption(ai::UnicodeString("return_value")),
            *form_return_data.GetSubList(ai::UnicodeString("l2a_item")));
    }
    else
    {
        l2a_error("Got unexpected ActionType");
    }
}

/**
 *
 */
void L2A::UI::Item::CreateNewItem(const L2A::UTIL::ParameterList& item_data_from_form)
{
    L2A::AI::SetUndoText(
        ai::UnicodeString("Undo Create LaTeX2AI Item"), ai::UnicodeString("Undo Create LaTeX2AI Item"));

    property_.SetFromParameterList(item_data_from_form);
    auto [latex_create_result, pdf_file] = L2A::LATEX::CreateLatexItem(property_);
    if (latex_create_result.result_ == L2A::LATEX::LatexCreationResult::Result::ok)
    {
        // Create the new item
        L2A::Item(new_item_insertion_point_, property_, pdf_file);

        // Everything worked fine, we can close the form now
        CloseForm();
    }
    else if (latex_create_result.result_ == L2A::LATEX::LatexCreationResult::Result::error_tex_code)
    {
        // Open the debug form for the user
        SetCloseOnFocus(true);
        L2A::GlobalPluginMutable().GetUiManager().GetDebugForm().OpenDebugForm(
            Debug::Action::create_item, latex_create_result);
    }
    else
    {
        // Ensure the form is closed on unexpected errors as well
        CloseForm();
    }
}

/**
 *
 */
void L2A::UI::Item::EditItem(const ai::UnicodeString& return_value, const L2A::UTIL::ParameterList& item_data_from_form)
{
    L2A::AI::SetUndoText(
        ai::UnicodeString("Undo Change LaTeX2AI Item"), ai::UnicodeString("Redo Change LaTeX2AI Item"));

    property_.SetFromParameterList(item_data_from_form);
    auto change_item_result = change_item_->Change(return_value, property_);
    if (change_item_result.result_ == L2A::ItemChangeResult::Result::ok ||
        change_item_result.result_ == L2A::ItemChangeResult::Result::cancel)
    {
        CloseForm();
        return;
    }
    else if (change_item_result.result_ == L2A::ItemChangeResult::Result::latex_error)
    {
        // Open the debug form for the user
        SetCloseOnFocus(true);
        L2A::GlobalPluginMutable().GetUiManager().GetDebugForm().OpenDebugForm(
            Debug::Action::edit_item, change_item_result.latex_creation_result_);
        return;
    }
}

/**
 *
 */
ASErr L2A::UI::Item::SendData()
{
    ASErr error = kNoErr;

    auto form_parameter_list = std::make_shared<L2A::UTIL::ParameterList>();

    // Set data about the item not contained in the item property
    ai::UnicodeString key_latex("latex_exists");
    if (action_type_ == ActionType::create_item)
    {
        form_parameter_list->SetOption(key_latex, false);
    }
    else if (action_type_ == ActionType::edit_item)
    {
        ai::UnicodeString key_boundary_box("boundary_box_state");
        form_parameter_list->SetOption(key_latex, true);

        L2A::GlobalMutable().logger_.push_back(
            ai::UnicodeString("L2A::UI::Item::SendData Line ") + L2A::UTIL::IntegerToString(__LINE__));

        if (change_item_->IsDiamond())
        {
            form_parameter_list->SetOption(key_boundary_box, ai::UnicodeString("diamond"));
        }
        else if (change_item_->IsStretched())
        {
            form_parameter_list->SetOption(key_boundary_box, ai::UnicodeString("stretched"));
        }
        else
        {
            form_parameter_list->SetOption(key_boundary_box, ai::UnicodeString("ok"));
        }
    }
    else
        l2a_error("Got unexpected ActionType");

    // Set behavior when pressing Enter
    form_parameter_list->SetOption(
        ai::UnicodeString("item_ui_finish_on_enter"), L2A::GlobalMutable().item_ui_finish_on_enter_);

    // Add the item property
    // TODO: Make sure we dont copy the pdf contents here
    form_parameter_list->SetSubList(ai::UnicodeString("LaTeX2AI_item"), property_.ToParameterList());

    // Send the data to the form
    SendDataWrapper(form_parameter_list, EVENT_TYPE_UPDATE);
}

/**
 *
 */
void L2A::UI::Item::SetCloseOnFocus(const bool value)
{
    auto form_parameter_list = std::make_shared<L2A::UTIL::ParameterList>();
    form_parameter_list->SetOption(ai::UnicodeString("close_on_focus"), value);
    SendDataWrapper(form_parameter_list, EVENT_TYPE_SET_CLOSE_ON_FOCUS);
}
