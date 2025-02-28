#! /bin/bash
# -----------------------------------------------------------------------------
# MIT License
#
# Copyright (c) 2020-2025 Ivo Steinbrecher
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# -----------------------------------------------------------------------------

# Get the path of the current script
SCRIPT_DIR="$(dirname "$0")"

# Set the key for the certificate
. ${SCRIPT_DIR}/SET_KEY.sh

# Remove the packed zxp file if it exists
ZXP_FILE=${SCRIPT_DIR}/latex2ai_ui.zxp
rm -f $ZXP_FILE

# Create the signed zxp file
${SCRIPT_DIR}/ZXPSignCmd-64bit -sign ${SCRIPT_DIR}/../../ui $ZXP_FILE ${SCRIPT_DIR}/latex2ai_certificate.p12 $LATEX2AI_CERTIFICATE_PASSWORD -tsa http://timestamp.digicert.com

# Unzip to folder
SIGNED_UI_FOLDER=${SCRIPT_DIR}/../release_files/com.latex2ai.latex2ai.${L2A_GIT_IDENTIFIER}
rm -rf $SIGNED_UI_FOLDER
mkdir $SIGNED_UI_FOLDER
unzip $ZXP_FILE -d $SIGNED_UI_FOLDER
rm -f $ZXP_FILE
