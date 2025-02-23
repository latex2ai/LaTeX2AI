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
 * \brief Form for debugging LaTeX errors
 */

#include "IllustratorSDK.h"

#include "l2a_ui_debug.h"

#include "l2a_ai_functions.h"
#include "l2a_execute.h"
#include "l2a_file_system.h"
#include "l2a_global.h"
#include "l2a_parameter_list.h"
#include "l2a_plugin.h"
#include "l2a_string_functions.h"


/**
 * \brief Set the names for item forms
 */
const std::string L2A::UI::Debug::FORM_NAME = "LaTeX2AI debug LaTeX code";
const std::string L2A::UI::Debug::FORM_ID = "com.adobe.illustrator.latex2aiui.dialog_debug";
const std::string L2A::UI::Debug::EVENT_TYPE_BASE = "com.adobe.csxs.events.latex2ai.debug";
const std::string L2A::UI::Debug::EVENT_TYPE_READY = L2A::UI::Debug::EVENT_TYPE_BASE + ".ready";
const std::string L2A::UI::Debug::EVENT_TYPE_OK = L2A::UI::Debug::EVENT_TYPE_BASE + ".ok";
const std::string L2A::UI::Debug::EVENT_TYPE_CANCEL = L2A::UI::Debug::EVENT_TYPE_BASE + ".cancel";
const std::string L2A::UI::Debug::EVENT_TYPE_OPEN_LOG = L2A::UI::Debug::EVENT_TYPE_BASE + ".open_log";
const std::string L2A::UI::Debug::EVENT_TYPE_CREATE_DEBUG = L2A::UI::Debug::EVENT_TYPE_BASE + ".create_debug";
const std::string L2A::UI::Debug::EVENT_TYPE_UPDATE = L2A::UI::Debug::EVENT_TYPE_BASE + ".update";


/**
 *
 */
L2A::UI::Debug::Debug() : FormBase(FORM_NAME, FORM_ID.c_str(), EVENT_TYPE_BASE), action_(Action::none)
{
    // If we don't do this this way, we get a compiler error
    std::vector<EventListenerData> event_listener_data = {
        {EVENT_TYPE_READY, CallbackHandler<Debug, &Debug::CallbackFormReady>()},                 //
        {EVENT_TYPE_OK, CallbackHandler<Debug, &Debug::CallbackReeditLatexCode>()},              //
        {EVENT_TYPE_CANCEL, CallbackHandler<Debug, &Debug::CallbackCancel>()},                   //
        {EVENT_TYPE_CREATE_DEBUG, CallbackHandler<Debug, &Debug::CallbackCreateDebugFolder>()},  //
        {EVENT_TYPE_OPEN_LOG, CallbackHandler<Debug, &Debug::CallbackOpenLog>()}                 //
    };
    event_listener_data_ = std::move(event_listener_data);
}

/**
 *
 */
void L2A::UI::Debug::OpenDebugForm(const Action action, L2A::LATEX::LatexCreationResult& latex_creation_result)
{
    action_ = action;
    latex_creation_result_ = std::move(latex_creation_result);
    LoadForm();
}

/**
 *
 */
void L2A::UI::Debug::CallbackFormReady(const csxs::event::Event* const eventParam) { SendData(); }

/**
 *
 */
void L2A::UI::Debug::CallbackReeditLatexCode(const csxs::event::Event* const eventParam)
{
    // We let the item form know that the user wants to reedit the item, i.e., the form is not closed once it comes back
    // into focus
    if (L2A::GlobalPluginMutable().GetUiManager().GetItemForm().GetActionType() != L2A::UI::Item::ActionType::none)
    {
        L2A::GlobalPluginMutable().GetUiManager().GetItemForm().SetCloseOnFocus(false);
    }
    CloseForm();
}

/**
 *
 */
void L2A::UI::Debug::CallbackCancel(const csxs::event::Event* const eventParam)
{
    if (L2A::GlobalPluginMutable().GetUiManager().GetItemForm().GetActionType() != L2A::UI::Item::ActionType::none)
    {
        // Send callback to close item other form
        L2A::GlobalPluginMutable().GetUiManager().GetItemForm().CloseForm();
    }
    CloseForm();
}

/**
 *
 */
void L2A::UI::Debug::CallbackOpenLog(const csxs::event::Event* const eventParam)
{
    L2A::UTIL::OpenFileWithDefaultApplication(latex_creation_result_.log_file_);
}

/**
 *
 */
void L2A::UI::Debug::CallbackCreateDebugFolder(const csxs::event::Event* const eventParam)
{
    auto app_context = L2A::GlobalPluginAppContext();
    ai::FilePath debug_directory = L2A::UTIL::GetPdfFileDirectory();
    debug_directory.AddComponent(ai::UnicodeString("debug"));

    if (L2A::UTIL::IsDirectory(debug_directory))
    {
        ai::UnicodeString form_string("");
        form_string += "The debug folder \"" + debug_directory.GetFullPath() +
                       "\" and its contents will be delete in this process. Do you want to continue?";
        if (!L2A::AI::YesNoAlert(form_string)) return;
    }

    // Clear the contents in the debug folder.
    L2A::UTIL::RemoveDirectoryAI(debug_directory, false);
    L2A::UTIL::CreateDirectoryL2A(debug_directory);

    // Create all relevant latex files in the debug folder
    auto copy_file_to_debug_directory = [&](const ai::FilePath& source_file_path) -> void
    {
        const auto name = source_file_path.GetFileName();
        ai::FilePath target_file = debug_directory;
        target_file.AddComponent(name);
        L2A::UTIL::CopyFileL2A(source_file_path, target_file);
    };

    copy_file_to_debug_directory(latex_creation_result_.tex_file_);
    copy_file_to_debug_directory(latex_creation_result_.tex_header_file_);

    sAIUser->MessageAlert("The debug files were created in \"" + debug_directory.GetFullPath() + "\"");
    L2A::UTIL::OpenFolder(debug_directory);
}

/**
 *
 */
ASErr L2A::UI::Debug::SendData()
{
    auto debug_parameter_list = std::make_shared<L2A::UTIL::ParameterList>();
    if (action_ == Action::create_item)
        debug_parameter_list->SetOption(ai::UnicodeString("action"), ai::UnicodeString("item_create"));
    else if (action_ == Action::edit_item)
        debug_parameter_list->SetOption(ai::UnicodeString("action"), ai::UnicodeString("item_edit"));
    else if (action_ == Action::redo_items)
        debug_parameter_list->SetOption(ai::UnicodeString("action"), ai::UnicodeString("redo_items"));
    else
        l2a_error("Got unknown action type");
    SendDataWrapper(debug_parameter_list, EVENT_TYPE_UPDATE);

    return kNoErr;
}
