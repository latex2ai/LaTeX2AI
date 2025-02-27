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
 * \brief Base class for a UI form
 */

#include "IllustratorSDK.h"

#include "l2a_ui_base.h"

#include "l2a_ai_functions.h"
#include "l2a_constants.h"
#include "l2a_string_functions.h"


/**
 *
 */
AIErr L2A::UI::FormBase::LoadForm()
{
    PlugPlugErrorCode result = LoadExtension();
    if (result != PlugPlugErrorCode_success)
    {
        ai::UnicodeString warning_string("");
        warning_string += "Could not open the LaTeX2AI user interface!\n";
        warning_string += "Please followed the installation instructions for the user interface stated on";
        warning_string += "\"https://github.com/LaTeX2AI/LaTeX2AI\"";
        L2A::AI::WarningAlert(warning_string);
    }
    return kNoErr;
}

/**
 *
 */
AIErr L2A::UI::FormBase::CloseForm()
{
    // Reset the state of the form
    ResetFormData();

    // Send callback to close the form
    const auto event_close = event_name_base_ + ".close";
    csxs::event::Event event = {event_close.c_str(), csxs::event::kEventScope_Application, "LaTeX2AI", NULL, ""};
    csxs::event::EventErrorCode result = htmlPPLib.DispatchEvent(&event);
    if (result != csxs::event::kEventErrorCode_Success)
    {
        l2a_error(form_name_ + " form could not be closed");
    }
    return kNoErr;
}

/**
 *
 */
void L2A::UI::FormBase::SetEventListeners(std::vector<EventListenerData>& event_listener_data)
{
    if (event_listener_data.size() == 0) l2a_error("SetEventListeners requires at least one event listener");
    event_listener_data_ = std::move(event_listener_data);
}

/**
 *
 */
csxs::event::EventErrorCode L2A::UI::FormBase::RegisterCSXSEventListeners()
{
    csxs::event::EventErrorCode result = csxs::event::kEventErrorCode_Success;
    for (auto& event_data : event_listener_data_)
    {
        result = htmlPPLib.AddEventListener(event_data.event_id_.c_str(), event_data.event_listener_, this);

        if (result != csxs::event::kEventErrorCode_Success)
        {
            return result;
        }
    }
    return result;
}

/**
 *
 */
csxs::event::EventErrorCode L2A::UI::FormBase::RemoveEventListeners()
{
    csxs::event::EventErrorCode result = csxs::event::kEventErrorCode_Success;
    for (auto& event_data : event_listener_data_)
    {
        result = htmlPPLib.RemoveEventListener(event_data.event_id_.c_str(), event_data.event_listener_, this);

        if (result != csxs::event::kEventErrorCode_Success)
        {
            return result;
        }
    }

    return result;
}

/**
 *
 */
void L2A::UI::FormBase::SendDataWrapper(
    const std::shared_ptr<L2A::UTIL::ParameterList>& form_data, const std::string& event_name)
{
    // Create the combines data object
    L2A::UTIL::ParameterList full_form_data;
    full_form_data.SetSubList(ai::UnicodeString("form_data"), form_data);

    // Add git hash to check that the form and plugin application have the same version
    full_form_data.SetOption(ai::UnicodeString("git_hash"), ai::UnicodeString(L2A_VERSION_GIT_SHA_HEAD_));

    // Get the string containing all data for the form and sent it
    std::string xml_string = L2A::UTIL::StringAiToStd(full_form_data.ToXMLString(ai::UnicodeString("full_data")));
    csxs::event::Event event = {
        event_name.c_str(), csxs::event::kEventScope_Application, "LaTeX2AI", NULL, xml_string.c_str()};
    csxs::event::EventErrorCode result = htmlPPLib.DispatchEvent(&event);
    if (result != csxs::event::kEventErrorCode_Success)
    {
        l2a_error("Data could not be sent to the " + form_name_ + " UI");
    }
}
