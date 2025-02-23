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
 * \brief Define a class that represents this plugin.
 */


#include "IllustratorSDK.h"

#include "l2a_plugin.h"

#include "AICSXS.h"
#include "AIMenuCommandNotifiers.h"
#include "SDKErrors.h"
#include "testing.h"

#include "l2a_ai_functions.h"
#include "l2a_constants.h"
#include "l2a_error.h"
#include "l2a_global.h"
#include "l2a_item.h"


/*
 * Free function to check if the document is a cloud document and throw a warning if this is the case
 */
bool IsActiveDocumentCloudDocumentWithWarning()
{
    const bool return_value = L2A::AI::IsActiveDocumentCloudDocument();
    if (return_value)
    {
        L2A::AI::MessageAlert(ai::UnicodeString("LaTeX2AI does not work with cloud documents!"));
    }
    return return_value;
}

/**
 */
Plugin* AllocatePlugin(SPPluginRef pluginRef) { return new L2APlugin(pluginRef); }

/**
 */
void FixupReload(Plugin* plugin) { L2APlugin::FixupVTable((L2APlugin*)plugin); }

/*
 */
L2APlugin::L2APlugin(SPPluginRef pluginRef)
    : Plugin(pluginRef),
      tool_handles_(0),
      notify_selection_changed_(nullptr),
      notify_document_save_(nullptr),
      notify_document_save_as_(nullptr),
      notify_active_doc_view_title_changed_(nullptr),
      notify_CSXS_plugplug_setup_complete_(nullptr),
      resource_manager_handle_(nullptr),
      ui_manager_(nullptr)
{
    // Set the name that of this plugin in Illustrator.
    strncpy(fPluginName, L2A_PLUGIN_NAME, kMaxStringLength);
}

/*
 */
ASErr L2APlugin::SetGlobal(Plugin* plugin)
{
    L2A::GLOBAL::_l2a_plugin = ((L2APlugin*)plugin);
    return kNoErr;
}

/*
 *
 */
ASErr L2APlugin::Notify(AINotifierMessage* message)
{
    ASErr error = kNoErr;

    try
    {
        if (message->notifier == notify_selection_changed_)
        {
            // Selection of art items changed in the document.

            // Update the annotator view bounds and the internal item vector.
            annotator_->InvalAnnotation();
            annotator_->ArtSelectionChanged();

            // Check if there is a single isolated l2a item.
            AIArtHandle placed_item;
            if (L2A::AI::GetSingleIsolationItem(placed_item))
            {
                if (!IsActiveDocumentCloudDocumentWithWarning())
                {
                    // Change the item
                    ui_manager_->GetItemForm().OpenEditItemForm(placed_item);
                }
            }
        }
        else if (message->notifier == notify_active_doc_view_title_changed_ ||
                 message->notifier == notify_document_save_ || message->notifier == notify_document_save_as_)
        {
            if (!L2A::AI::IsActiveDocumentCloudDocument())
            {
                L2A::AI::UndoActivate();
                L2A::CheckItemDataStructure();

                // Update the annotator view bounds and the internal item vector.
                annotator_->InvalAnnotation();
                annotator_->ArtSelectionChanged();
            }
        }
        else if (message->notifier == notify_CSXS_plugplug_setup_complete_)
        {
            ui_manager_->RegisterCSXSEventListeners();
        }
    }
    catch (L2A::ERR::Exception&)
    {
        sAIUser->MessageAlert(ai::UnicodeString("L2APlugin::Notify Error caught."));
        // TODO: check this error message here
        error = 1;
    }

    return error;
}

/*
 */
ASErr L2APlugin::Message(char* caller, char* selector, void* message)
{
    ASErr error = kNoErr;
    try
    {
        // Call the message method of the base class to access mouse events e.t.c.
        error = Plugin::Message(caller, selector, message);

        if (error == kUnhandledMsgErr)
        {
            if (strcmp(caller, kCallerAIAnnotation) == 0)
            {
                auto* annotator_message = (AIAnnotatorMessage*)message;
                if (annotator_message == nullptr)
                {
                    // In case the annotator message pointer is null, we don't do anything here.
                }
                else
                {
                    if (strcmp(selector, kSelectorAIDrawAnnotation) == 0)
                        // Draw the l2a annotator.
                        annotator_->Draw(annotator_message);
                    else if (strcmp(selector, kSelectorAIInvalAnnotation) == 0)
                    {
                        // Invalidate the annotator.
                        // TODO: maybe this function call can be removed. Check if it is called at all / what the given
                        // invalidate box actually is.

                        // In some calls, the pointer to the invalidation rectangle is null, in these cases we
                        // invalidate the whole document view.
                        auto* artwork_bounds = annotator_message->invalidationRects;
                        if (artwork_bounds == nullptr)
                        {
                            annotator_->InvalAnnotation();
                        }
                        else
                        {
                            annotator_->InvalAnnotation(*artwork_bounds);
                        }
                    }
                }
            }
        }
        else
            aisdk::check_ai_error(error);
    }
    catch (ai::Error& ex)
    {
        error = ex;
    }
    catch (...)
    {
        error = kCantHappenErr;
    }
    return error;
}

/*
 */
ASErr L2APlugin::StartupPlugin(SPInterfaceMessage* message)
{
    ASErr error = kNoErr;
    try
    {
        ai::int32 pluginOptions = 0;

        // We need to call the base startup method to avoid memory leaks when closing the plugin.
        error = Plugin::StartupPlugin(message);
        aisdk::check_ai_error(error);

        error = sAIPlugin->GetPluginOptions(message->d.self, &pluginOptions);
        aisdk::check_ai_error(error);

        error = sAIPlugin->SetPluginOptions(message->d.self, pluginOptions | kPluginWantsResultsAutoSelectedOption);
        aisdk::check_ai_error(error);

        // Set the global l2a object. This object should only be used with the Get functions in L2A.
        L2A::GLOBAL::_l2a_global = new L2A::GLOBAL::Global();

        // Register relevant LaTeX2AI functions
        error = AddNotifier(message);
        aisdk::check_ai_error(error);
        error = AddTools(message);
        aisdk::check_ai_error(error);
        error = AddAnnotator(message);
        aisdk::check_ai_error(error);

#ifdef _DEBUG
        // In the debug mode perform all unit tests at startup.
        static const bool print_status = false;
        L2A::TEST::TestingMain(print_status);
#endif

        // Lock the plug-in as we register callbacks in PlugPlug Setup
        // TODO check if this is needed
        ui_manager_ = std::make_unique<L2A::UI::Manager>();
        error = Plugin::LockPlugin(true);
        aisdk::check_ai_error(error);
    }
    catch (ai::Error& ex)
    {
        error = ex;
    }
    catch (L2A::ERR::Exception&)
    {
        sAIUser->MessageAlert(ai::UnicodeString("Error wile setting up LaTeX2AI"));
        // TODO: Should we return an error here?
    }
    catch (...)
    {
        error = kCantHappenErr;
    }
    return error;
}

/*
 */
ASErr L2APlugin::ShutdownPlugin(SPInterfaceMessage* message)
{
    ASErr error = kNoErr;
    try
    {
        // If it was created, delete the global object.
        if (L2A::GLOBAL::_l2a_global != nullptr) delete L2A::GLOBAL::_l2a_global;

        // Remove the UI event listeners
        ui_manager_->RemoveEventListeners();

        // Dereference the annotator and the ui manager -> the objects will be delete here, otherwise we would have a
        // memory leak later
        annotator_ = nullptr;
        ui_manager_ = nullptr;

        // We need to call the base shutdown method to avoid memory leaks when closing the plugin.
        error = Plugin::ShutdownPlugin(message);
        aisdk::check_ai_error(error);
    }
    catch (ai::Error& ex)
    {
        error = ex;
    }
    catch (...)
    {
        error = kCantHappenErr;
    }
    return error;
}

/*
 */
ASErr L2APlugin::ToolMouseDown(AIToolMessage* message)
{
    ASErr error = kNoErr;

    if (message->tool == this->tool_handles_[0])
    {
        // Selected tool is item create.
        try
        {
            if (!IsActiveDocumentCloudDocumentWithWarning())
            {
                if (annotator_->IsArtHit())
                {
                    ui_manager_->GetItemForm().OpenEditItemForm(annotator_->GetArtHit());
                }
                else
                {
                    // Check if the current insertion point is locked.
                    if (!L2A::AI::GetLockedInsertionPoint())
                    {
                        ui_manager_->GetItemForm().OpenCreateItemForm(message->cursor);
                    }
                    else
                    {
                        sAIUser->MessageAlert(
                            ai::UnicodeString("You tried to create a LaTeX2AI item inside a locked or hidden layer / "
                                              "group. This is not possible."));
                    }
                }
            }
        }
        catch (L2A::ERR::Exception&)
        {
            sAIUser->MessageAlert(ai::UnicodeString("L2APlugin::ToolMouseDown Error caught."));
        }
    }

    return error;
}

/*
 */
ASErr L2APlugin::AddTools(SPInterfaceMessage* message)
{
    AIErr error = kNoErr;

    // Set the number of tools.
#ifdef _DEBUG
    static const unsigned int n_tools = 5;
#else
    static const unsigned int n_tools = 4;
#endif

    // Data used to add tool.
    AIAddToolData data;

    // Define the name and icon for the tools.
    char toolGroupName[256];
    std::vector<std::string> tool_title = {//
        "LaTeX2AI create edit mode",       //
        "LaTeX2AI redo items",             //
        "LaTeX2AI options",                //
        "LaTeX2AI save to PDF",            //
        "Perform LaTeX2AI tests"};
    std::vector<std::string> tool_tip = {                                                                      //
        "LaTeX2AI create edit mode: Create a new LaTeX2AI item, or edit an existing item by clicking on it",   //
        "LaTeX2AI redo items: Redo multiple items in this document",                                           //
        "LaTeX2AI options: Set local and global options for LaTeX2AI",                                         //
        "LaTeX2AI save to PDF: Save a copy of the current Illustrator file to a pdf file with the same name",  //
        "Perform LaTeX2AI framework test suite"};

    // Define icons.
    std::vector<ai::uint32> light_icon_id = {TOOL_ICON_CREATE_LIGHT_ID, TOOL_ICON_REDO_LIGHT_ID,
        TOOL_ICON_OPTIONS_LIGHT_ID, TOOL_ICON_SAVE_AS_PDF_LIGHT_ID, TOOL_ICON_TESTING_LIGHT_ID};
    std::vector<ai::uint32> dark_icon_id = {TOOL_ICON_CREATE_DARK_ID, TOOL_ICON_REDO_DARK_ID, TOOL_ICON_OPTIONS_DARK_ID,
        TOOL_ICON_SAVE_AS_PDF_DARK_ID, TOOL_ICON_TESTING_DARK_ID};

    // Create all tools.
    tool_handles_.resize(n_tools);
    for (short i = 0; i < n_tools; i++)
    {
        // Define the name and icon for the tool.
        data.title = ai::UnicodeString(tool_title[i]);
        data.tooltip = ai::UnicodeString(tool_tip[i]);
        data.normalIconResID = light_icon_id[i];
        data.darkIconResID = dark_icon_id[i];
        data.iconType = ai::IconType::kSVG;

        if (i == 0)
        {
            // The first tool creates a new tool palette.
            strcpy(toolGroupName, tool_title[i].c_str());
            // New group on tool palette.
            data.sameGroupAs = kNoTool;
        }
        else
        {
            error = sAITool->GetToolNumberFromName(toolGroupName, &data.sameGroupAs);
            l2a_check_ai_error(error);
        }

        // Each tool icon is on its own.
        data.sameToolsetAs = kNoTool;

        // Add the tool.
        // We want to track the cursor with each tool. Otherwise, we can not deselect some tools when using them as
        // "buttons".
        error = sAITool->AddTool(
            message->d.self, tool_title[i].c_str(), data, kToolWantsToTrackCursorOption, &tool_handles_[i]);
        l2a_check_ai_error(error);
    }

    return error;
}

/**
 *
 */
ASErr L2APlugin::AddAnnotator(SPInterfaceMessage* message)
{
    ASErr result = kNoErr;
    try
    {
        annotator_ = std::make_unique<L2A::Annotator>(message);
    }
    catch (ai::Error& ex)
    {
        result = ex;
    }
    catch (...)
    {
        result = kCantHappenErr;
    }
    return result;
}

/*
 *
 */
ASErr L2APlugin::AddNotifier(SPInterfaceMessage* message)
{
    ASErr result = kNoErr;
    try
    {
        result = sAINotifier->AddNotifier(
            fPluginRef, L2A_PLUGIN_NAME, kAIArtSelectionChangedNotifier, &notify_selection_changed_);
        aisdk::check_ai_error(result);
        result =
            sAINotifier->AddNotifier(fPluginRef, L2A_PLUGIN_NAME, kAISaveCommandPreNotifierStr, &notify_document_save_);
        aisdk::check_ai_error(result);
        result = sAINotifier->AddNotifier(
            fPluginRef, L2A_PLUGIN_NAME, kAISaveAsCommandPostNotifierStr, &notify_document_save_as_);
        aisdk::check_ai_error(result);
        result = sAINotifier->AddNotifier(
            fPluginRef, L2A_PLUGIN_NAME, kAIActiveDocViewTitleChangedNotifier, &notify_active_doc_view_title_changed_);
        aisdk::check_ai_error(result);
        result = sAINotifier->AddNotifier(message->d.self, L2A_PLUGIN_NAME, kAICSXSPlugPlugSetupCompleteNotifier,
            &notify_CSXS_plugplug_setup_complete_);
        aisdk::check_ai_error(result);
    }
    catch (ai::Error& ex)
    {
        result = ex;
    }
    catch (...)
    {
        result = kCantHappenErr;
    }
    return result;
}

/**
 *
 */
ASErr L2APlugin::SelectTool(AIToolMessage* message)
{
    AIErr error = kNoErr;

    if (message->tool == this->tool_handles_[0])
    {
        // Create / edit tool is selected.
        // Activate the annotator.
        annotator_->SetAnnotatorActive();
    }
    else if (message->tool == this->tool_handles_[1] && L2A::AI::GetDocumentCount() > 0)
    {
        if (!IsActiveDocumentCloudDocumentWithWarning())
        {
            ui_manager_->GetRedoForm().OpenRedoForm();
        }
    }
    else if (message->tool == this->tool_handles_[2])
    {
        if (!IsActiveDocumentCloudDocumentWithWarning())
        {
            ui_manager_->GetOptionsForm().OpenOptionsForm();
        }
    }
    else if (message->tool == this->tool_handles_[3] && L2A::AI::GetDocumentCount() > 0)
    {
        L2A::AI::SaveToPDF();

        // Deselect all so a call to the plugin will be given, where the tools can be deselected.
        error = sAIMatchingArt->DeselectAll();
        l2a_check_ai_error(error);
    }
#ifdef _DEBUG
    else if (message->tool == this->tool_handles_[4])
    {
        // Test the LaTeX2AI framework.
        L2A::TEST::TestFramework();
    }
#endif

    return error;
}

/**
 *
 */
ASErr L2APlugin::DeselectTool(AIToolMessage*)
{
    ASErr result = kNoErr;
    try
    {
        // Deactivate the annotator
        annotator_->SetAnnotatorInactive();
    }
    catch (ai::Error& ex)
    {
        result = ex;
    }
    catch (...)
    {
        result = kCantHappenErr;
    }
    return result;
}

/**
 *
 */
ASErr L2APlugin::TrackToolCursor(AIToolMessage* message)
{
    AIErr error = kNoErr;

    if (message->tool == this->tool_handles_[0])
    {
        // Create edit mode is active

        // Check if cursor is over an art item
        ai::int32 cursor_id;
        if (annotator_->CheckForArtHit(message))
            cursor_id = CURSOR_ICON_EDIT;
        else
        {
            if (L2A::AI::GetLockedInsertionPoint())
                cursor_id = CURSOR_ICON_LOCKED;
            else
                cursor_id = CURSOR_ICON_CREATE;
        }
        error = sAIUser->SetSVGCursor(cursor_id, resource_manager_handle_);
        l2a_check_ai_error(error);
    }
    else if (message->tool == this->tool_handles_[1] || message->tool == this->tool_handles_[2] ||
             message->tool == this->tool_handles_[3])
    {
        // A LaTeX2AI UI is opened - we want to deselect the tool in this case, we select the default AI selection tool
        error = sAITool->SetSelectedToolByName(kSelectTool);
        l2a_check_ai_error(error);
    }

    return error;
}

/**
 *
 */
ASErr L2APlugin::PostStartupPlugin()
{
    AIErr result = kNoErr;
    result = sAIUser->CreateCursorResourceMgr(fPluginRef, &resource_manager_handle_);
    return result;
}
