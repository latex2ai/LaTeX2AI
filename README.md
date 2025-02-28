![LaTeX2AI](/doc/logo/logo_black.png?raw=true)

LaTeX2AI is a plugin for Adobe Illustrator (MacOS and Windows) that enables the use of editable text labels typeset in LaTeX inside an Illustrator document.

# License & How to cite

LaTeX2AI is under the MIT license, see [./LICENSE](LICENSE).
If you use LaTeX2AI to create figures for your work, please acknowledge it with a link to the GitHub repository.
For example:

-   Sketches in this work have been created using the Adobe Illustrator plug-in LaTeX2AI ([https://github.com/LaTeX2AI/LaTeX2AI](https://github.com/LaTeX2AI/LaTeX2AI)).

Feel free to leave a :star: on [GitHub](https://github.com/LaTeX2AI/LaTeX2AI).
You can also add your work to [Work that uses LaTeX2AI](#work-that-uses-latex2ai).

# Installation

### Requirements

The following software dependencies are required to run LaTeX2AI:

-   A LaTeX compiler such as [TeX Live](https://www.tug.org/texlive) or [MiKTeX](https://miktex.org)
-   [Ghost script](https://www.ghostscript.com)

### Windows

1. Download LaTeX2AI from the [GitHub release page](https://github.com/LaTeX2AI/LaTeX2AI/releases)
1. Unzip the `.zip` file
1. Copy the user interface folder `com.latex2ai.latex2ai`

    - LaTeX2AI installation only for **_current_ user**:
        - Copy `com.latex2ai.latex2ai` to `C:\Users\<USERNAME>\AppData\Roaming\Adobe\CEP\extensions` (the directory might have to be created)
    - LaTeX2AI installation for **_all_ users** (requires administrator privileges):
        - Copy `com.latex2ai.latex2ai` to `C:\Program Files\Common Files\Adobe\CEP\extensions\`

1. Copy the plugin

    - LaTeX2AI installation only for **_current_ user**:
        - Copy `WIN/LaTeX2AI.aip` to an arbitrary directory. This directory has to be set as the Adobe Illustrator Plugin directory via `Edit/Preferences/Plug-ins & Scratch Disks.../Additional Plug-ins Folder/`
    - LaTeX2AI installation for **_all_ users** (requires administrator privileges):
        - Copy `WIN/LaTeX2AI.aip` to `C:\Program Files\Adobe\Adobe Illustrator <YOUR VERSION>\Plug-ins\`

1. After a restart of Adobe Illustrator, you can display the LaTeX2AI tools with `Window/Toolbars/Advanced`.

To uninstall LaTeX2AI, delete the files you copied.

### macOS

1. Download LaTeX2AI from the [GitHub release page](https://github.com/LaTeX2AI/LaTeX2AI/releases)
1. Unzip the file
1. Copy the user interface folder `com.latex2ai.latex2ai`

    - LaTeX2AI installation only for **_current_ user**:
        - Copy `com.latex2ai.latex2ai` to `~/Library/Application Support/Adobe/CEP/extensions/`
    - LaTeX2AI installation for **_all_ users** (requires administrator privileges):
        - Copy `com.latex2ai.latex2ai` to `/Library/Application Support/Adobe/CEP/extensions/`

1. Copy the plugin

    - LaTeX2AI installation only for **_current_ user**:
        - Copy `macOS/LaTeX2AI.aip` to an arbitrary directory. This directory has to be set as the Adobe Illustrator Plugin directory via `Edit/Preferences/Plug-ins & Scratch Disks.../Additional Plug-ins Folder/`
    - LaTeX2AI installation for **_all_ users** (requires administrator privileges):
        - Copy `macOS/LaTeX2AI.aip` to `/Applications/Adobe Illustrator <YOUR VERSION>/Plug-ins/`

1. After a restart of Adobe Illustrator, you can display the LaTeX2AI tools with `Window/Toolbars/Advanced`.

    > Depending on your system settings you might get the following error message when starting Illustrator
    >
    > <img src="/doc/images/macos_permission_error.png?raw=true" alt="macOS permission error" height="350" title="Create / Edit">
    >
    > This can be resolved by explicitly allowing the gatekeeper to run LaTeX2AI ([see also this thread](https://apple.stackexchange.com/questions/58509/how-can-i-open-a-single-app-by-an-unidentified-developer-without-allowing-all)). To do so, open the terminal and type:
    >
    > ```bash
    > xattr -d com.apple.quarantine <PATH TO LaTeXAI.aip>
    > ```
    >
    > If you have installed LaTeX2AI for _all_ users you need to run this command with administrator privileges:
    >
    > ```bash
    > sudo xattr -d com.apple.quarantine <PATH TO LaTeXAI.aip>
    > ```

To uninstall LaTeX2AI, delete the files you copied.

# How to use LaTeX2AI

LaTeX2AI adds four buttons to the main toolbar:

-   ![Create / Edit](/doc/images/tool_create.png?raw=true "Create / Edit") **Create / Edit**: Edit an existing label by clicking on it, or creating a new one by clicking somewhere in the document.
-   ![Redo items](/doc/images/tool_redo.png?raw=true "Redo labels") **Redo LaTeX2AI labels**: This allows for the LaTeX recompilation and/or scaling reset of all existing LaTeX2AI labels.
-   ![LaTeX2AI options](/doc/images/tool_options.png?raw=true "LaTeX2AI options") **LaTeX2AI options**: Open a form where the global LaTeX2AI options can be set. Also the LaTeX header can be opened in an external application.
-   ![Save document as PDF](/doc/images/tool_save_as_pdf.png?raw=true "Save document as PDF") **Save as PDF**: Save the current `.ai` document as a `.pdf` document with the same name. The LaTeX2AI labels are included into the created `.pdf` document.

These buttons are the main way of interacting with LaTeX2AI.
Additionally, double clicking on a LaTeX2AI label will enable the edit mode for that label.

## Create / Edit a LaTeX2AI label

The following form appears when creating or editing LaTeX2AI labels:

![Create / Edit](/doc/images/create_edit_ui.png?raw=true "The LaTeX2aI Create / Edit form")

### Placement

This option defines how a label behaves when it its size changes.
Take for example the well-known formula `$\sum_{k=0}^{\infty}\frac{x^k}{k!}$` which is placed inside a rectangle:

![Placement small](/doc/images/placement_small.png?raw=true)

The green box indicates the size of the LaTeX2I label and the dot describes the placement of the label. If the size changes due to a change in the LaTeX code, the position of this dot relative to the label will stay the same.
If the previous label is now changed to `\displaystyle` we get the following result:

![Placement large](/doc/images/placement_large.png?raw=true)

The size of the label changed due to a change in the underlying LaTeX code, but the position (the indicated dot) stayed the same.

#### Baseline placement

Additionally, there is the possibility of a _baseline_ placement.
This will result in a label where the baseline is exactly in the vertical center of the label.
This label can now be easily adjusted and snapped to align with another baseline label or Illustrator text:

![Placement baseline](/doc/images/placement_baseline.png?raw=true)

## Item scaling behavior

Per default, LaTeX2AI places the LaTeX labels inside Illustrator with a scale of 1:1, i.e., 1pt in the LaTeX label is 1pt in the Illustrator document.
For example, if the LaTeX item is created with a font size of 12pt the font will match Illustrator text with 12pt.
The user can resize the labels like any other Illustrator object, however, be aware that every time the LaTeX code of a label changes, the scale is reset to 1:1.

The recommended way of using LaTeX2AI is to always have items at a scale of 1:1.
By doing so, the exported Illustrator document can be simply included into a LaTeX document with the `\includegraphics` option `scale=1`, and the font size of the labels in the figures will exactly match the font size of the document (assuming that the header options are the same).
With the LaTeX2AI tool **Redo items**, one can easily reset the scaling of all LaTeX2AI items in the document.

## LaTeX header

LaTeX2AI assumed that all Illustrator files in the same directory use the same LaTeX header `LaTeX2AI_header.tex` (if no one exits in the directory, it will be created the first time it is needed).
This header can be edited to include packages and macros needed for the labels.

## File structure

The `.pdf` files for the LaTeX labels are stored in the `links` subfolder of the document directory.
It is not required to keep track of the files in the `links` folder, LaTeX2AI manages and deletes unused label files.

# Known issues

## Creative Cloud documents

LaTeX2AI currently does not work with _Creative Cloud documents_. However, if the document is stored in the _Creative Cloud Files_ folder on the disk, LaTeX2AI should work as expected.

# Work that uses LaTeX2AI

An incomplete list of work that uses LaTeX2AI

-   Toenniessen, F.: Die Homotopie der Sphären - Eine Einführung in Spektralsequenzen, Lokalisierungen und Kohomologie-Operationen, Springer Verlag Heidelberg, 2023.
-   Steinbrecher, I., Popp, A., Meier, C.: Consistent coupling of positions and rotations for embedding 1D Cosserat beams into 3D solid volumes. Comput Mech (2021), [Open Access](https://doi.org/10.1007/s00466-021-02111-4)
-   Steinbrecher, I., Mayr, M., Grill, M.J., Kremheller, J., Meier, C., Popp, A.: A mortar-type finite element approach for embedding 1D beams into 3D solid volumes, Comput Mech (2020), 66(6):1377-1398, [Open Access](https://doi.org/10.1007/s00466-020-01907-0)
-   Steinbrecher, I., Humer, A., Vu-Quoc, L.: On the numerical modeling of sliding beams: A comparison of different approaches, Journal of Sound and Vibration, 408:270-290, [Open Access](https://doi.org/10.1016/j.jsv.2017.07.010)

# Contributing

If you are interested in contributing to LaTeX2AI, we welcome your collaboration.
For general questions, feature request and bug reports please open an [issue](https://github.com/LaTeX2AI/LaTeX2AI/issues).

If you contribute actual code, fork the repository and make the changes in a feature branch.
Depending on the topic and amount of changes you also might want to open an [issue](https://github.com/LaTeX2AI/LaTeX2AI/issues).
To merge your changes into the main LaTeX2AI repository, create a pull request to the `develop` branch (this branch will be merged into `main` with the next release).
A few things to keep in mind:

-   Compile a debug build and run the framework tests in Illustrator ![Framework tests](/doc/images/tool_testing.png?raw=true "Framework tests").
    You might also consider adding tests for your changes (`./src/tests`).
-   Run the `python3` script `./scripts/check_license.py` to ensure that all added source files have the correct license header.
-   LaTeX2AI uses `clang-format` to format the C++ code.
    Make sure to apply clang format to the changed source files:
    -   With the Visual Studio solution in the repository this can be done with `Crtl-K` followed by `Ctrl-D`.
    -   On macOS you can use the following command (run in the root directory of LaTeX2AI):
        ```bash
        find src -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
        ```
-   Add a short description of your changes to the [Changelog](doc/CHANGELOG.md).
-   Feel free to add yourself to the [./CONTRIBUTORS](CONTRIBUTORS) file.

# Build LaTeX2AI from source

Instructions on how to build LaTeX2AI from source can be found [here](doc/BUILD_FROM_SOURCE.md)

# Changelog

A detailed changelog can be found [here](doc/CHANGELOG.md)
