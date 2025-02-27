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
 * \brief Errors for l2a applications.
 */


#include "IllustratorSDK.h"

#include "l2a_error.h"

#include "l2a_constants.h"
#include "l2a_string_functions.h"


#if defined(WIN_ENV) && defined(_DEBUG)
#include "StackWalker.h"

/**
 * \brief User class to obtain a vector with the current stack.
 */
class MyStackWalker : public StackWalker
{
   public:
    /**
     * \brief Constructor.
     */
    MyStackWalker() : StackWalker(), stack_lines_() {}

    /**
     * \brief Return a string with the stack information.
     */
    ai::UnicodeString GetStack() const
    {
        ai::UnicodeString stack_string("");
        for (unsigned int i = 0; i < stack_lines_.size(); i++)
        {
            stack_string += stack_lines_[i];
            if (i < stack_lines_.size() - 1) stack_string += "\n";
        }
        return stack_string;
    }

   protected:
    /**
     * \brief This method is called for each line in the stack.
     *
     * Here we check if the line is of interest to us and save it in the stack vector.
     */
    virtual void OnOutput(LPCSTR szText)
    {
        std::string stack_line(szText);
        if (stack_line.find("latex2ai") != std::string::npos && stack_line.find("StackWalker") == std::string::npos)
            stack_lines_.push_back(ai::UnicodeString(szText));
    }

   private:
    //! Vector with the relevant stack lines.
    std::vector<ai::UnicodeString> stack_lines_;
};
#endif

/**
 *
 */
L2A::ERR::Exception::Exception(const char* __file__, const int __line__, const char* __function__,
    const ai::UnicodeString& error_string, const bool display_error_message)
    : ExceptionBase()
{
    ai::UnicodeString full_error_string(
        "--------------------------------------------------------------------------------\n");
    full_error_string += "LaTeX2AI Error! If this happens, please open an issue under\n";
    full_error_string += "github.com/LaTeX2AI/LaTeX2AI/issues\n";
    full_error_string += "--------------------------------------------------------------------------------\n";
    full_error_string += "LaTeX2AI version:\n";
    full_error_string += L2A_VERSION_STRING_;
    full_error_string += "\nLaTeX2AI git sha:\n";
    full_error_string += L2A_VERSION_GIT_SHA_HEAD_;
    full_error_string += "\n\nIllustrator Version:\n";
    full_error_string += ai::UnicodeString(L2A::UTIL::IntegerToHexString(kPluginInterfaceVersion));
    full_error_string += "\n--------------------------------------------------------------------------------\n";
#if defined(WIN_ENV) && defined(_DEBUG)
    full_error_string += "Stack:\n";
    full_error_string += "--------------------------------------------------------------------------------\n";
    MyStackWalker sw;
    sw.ShowCallstack();
    full_error_string += sw.GetStack();
    full_error_string += "\n--------------------------------------------------------------------------------\n";
#endif
    full_error_string += "Error thrown at:\n";
    full_error_string += "--------------------------------------------------------------------------------\n";
    full_error_string += ai::UnicodeString(__file__);
    full_error_string += "::";
    char buff[100];
    snprintf(buff, sizeof(buff), "%d", __line__);
    full_error_string += ai::UnicodeString(buff);
    full_error_string += "\n";
    full_error_string += ai::UnicodeString(__function__);
    full_error_string += "\n--------------------------------------------------------------------------------\n\n";
    full_error_string += error_string;

    if (display_error_message) sAIUser->MessageAlert(full_error_string);
}

/**
 *
 */
ai::UnicodeString L2A::ERR::AIErrorCodeToString(const int& err)
{
    int error_local = err;
    std::vector<char> chars(4);
    for (unsigned int i = 0; i < 4; i++)
    {
        // Get the first 8 bits.
        chars[3 - i] = error_local & 0xFF;

        // Shift by 8 bits.
        error_local >>= 8;
    }

    // Return the Illustrator error code.
    return ai::UnicodeString(chars.data(), 4);
}

/**
 *
 */
ai::UnicodeString L2A::ERR::AIErrorCodeToErrorString(const int& err)
{
    ai::UnicodeString error_string("Ilustrator Error.\nError code (int): ");
    error_string += L2A::UTIL::IntegerToString(err);
    error_string += "\nError code: ";
    error_string += AIErrorCodeToString(err);
    return error_string;
}
