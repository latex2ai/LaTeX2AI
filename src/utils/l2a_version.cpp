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
 * \brief Functions to check the version of LaTeX2AI.
 */


#include "IllustratorSDK.h"

#include "l2a_version.h"

#include "json.hpp"

#include "l2a_constants.h"
#include "l2a_error.h"
#include "l2a_execute.h"
#include "l2a_file_system.h"
#include "l2a_string_functions.h"


/**
 *
 */
semver::version L2A::UTIL::ParseVersion(const std::string& version_string)
{
    std::string string_to_parse = version_string;

    // The github versions have a prefix "v" -> remove this.
    if (version_string.at(0) == 'v') string_to_parse = string_to_parse.substr(1, string_to_parse.size() - 1);

    return semver::version::parse(string_to_parse);
}

/**
 *
 */
void L2A::UTIL::CheckGithubVersion()
{
    try
    {
        // Get the packages in the GitHub repository.
        ai::UnicodeString command("curl -s https://api.github.com/repos/LaTeX2AI/LaTeX2AI/releases");
        ai::UnicodeString result;

        auto command_result = L2A::UTIL::ExecuteCommandLine(command);
        std::string curl_output = L2A::UTIL::StringAiToStd(command_result.output_);
        if (curl_output == "") return;

        // Convert the string to a json object.
        using json = nlohmann::json;
        auto github_releases = json::parse(curl_output);

        // Get the version tags.
        std::vector<semver::version> github_versions;
        for (auto& [key, value] : github_releases.items())
        {
            if (value.contains("tag_name"))
            {
                const auto tag_version = L2A::UTIL::ParseVersion(value["tag_name"].get<std::string>());

                // Only add versions that are not pre release versions
                if (!tag_version.is_prerelease())
                {
                    github_versions.push_back(tag_version);
                }
            }
        }

        // If for some reasons no version could be found, return here.
        if (github_versions.size() == 0) l2a_error_quiet(ai::UnicodeString("Could not retrieve github versions."));

        // Get the current version.
        auto& newest_version = *(std::max_element(std::begin(github_versions), std::end(github_versions)));
        const auto current_version = L2A::UTIL::ParseVersion(L2A_VERSION_STRING_);
        if (current_version < newest_version)
        {
            ai::UnicodeString message_string("The new LaTeX2AI version v");
            message_string += newest_version.str();
            message_string +=
                " is available at GitHub (https://github.com/LaTeX2AI/LaTeX2AI/releases). The currently used "
                "version is v";
            message_string += current_version.str() + ".";
            sAIUser->MessageAlert(message_string);
        }
    }
    catch (...)
    {
#ifdef _DEBUG
        sAIUser->MessageAlert(ai::UnicodeString("Error in CheckGithubVersion"));
#endif
    }
}
