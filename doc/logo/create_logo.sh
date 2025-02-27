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

# Create the logo and icon pdf
pdflatex logo_icon.tex

# Convert the pdf into pngs
# -r = [png pixel size] / [size in mm] / 10 / 2.54
gs -dFirstPage=1 -dLastPage=1 -sDEVICE=pngalpha -dTextAlphaBits=4 -r2442 -o logo_black.png logo_icon.pdf
gs -dFirstPage=2 -dLastPage=2 -sDEVICE=pngalpha -dTextAlphaBits=4 -r2442 -o logo_color.png logo_icon.pdf
gs -dFirstPage=3 -dLastPage=3 -sDEVICE=pngalpha -dTextAlphaBits=4 -r2442 -o logo_square.png logo_icon.pdf
gs -dFirstPage=4 -dLastPage=4 -sDEVICE=pngalpha -dTextAlphaBits=4 -r1250 -o icon_256.png logo_icon.pdf
gs -dFirstPage=5 -dLastPage=5 -sDEVICE=pngalpha -dTextAlphaBits=4 -r2442 -o logo_square_bw.png logo_icon.pdf

# Create the image placement
pdflatex item_placement.tex

# Convert the pdf into png
gs -dFirstPage=1 -dLastPage=1 -sDEVICE=pngalpha -dTextAlphaBits=4 -r650 -o ../../ui/html/item_placement_light.png item_placement.pdf
gs -dFirstPage=2 -dLastPage=2 -sDEVICE=pngalpha -dTextAlphaBits=4 -r650 -o ../../ui/html/item_placement_dark.png item_placement.pdf
