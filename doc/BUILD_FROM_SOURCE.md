# Build LaTeX2AI from source

## Requirements

The additional requirements to build LaTeX2AI from source are:

-   A C++ compiler has to be available on the system. This repository includes a Visual Studio 2022 solution for Windows and a Xcode project for macOS.
-   `python3` and `git` have to be available on the system.

## Getting started

1. Download and unpack the [Adobe Illustrator SDK](https://console.adobe.io/downloads)

1. Go to the sample code folder `<Adobe Illustrator SDK>/sample code` and run
    ```
    git clone https://github.com/LaTeX2AI/LaTeX2AI.git
    ```
1. Go to `<Adobe Illustrator SDK>/sample code/LaTeX2AI` and run
    ```
    git submodule update --init
    ```

## Windows

1.  Environment variables:

    -   `PYTHON_EXE` has to point to a valid `python3` executable on the system.
    -   `ILLUSTRATOR_EXE` has to point to the illustrator executable on the system (this path is used for debugging).

1.  Open `<Adobe Illustrator SDK>/sample code/LaTeX2AI/LaTeX2AI.sln` with Visual Studio, select the build type (_Release_ or _Debug_) and compile the solution.
    It might be necessary to change the _Windows SDK_ version for `LaTeX2AI` to match the one available on your system.
1.  The compiled binary is located at `<Adobe Illustrator SDK>/sample code/output/win/x64/<Debug/Release>`.
    This directory has to be set as the Plugin directory in Illustrator.
1.  You have to set a link to the UI folder so Illustrator can find the user interface:
    ```
    mklink /D "C:\Users\<USERNAME>\AppData\Roaming\Adobe\CEP\extensions\com.latex2ai.latex2ai" "<Adobe Illustrator SDK>\samplecode\latex2ai\ui"
    ```
1.  Debugging of unsigned extensions has to be turned on ([see the Adobe CEP cookbook](https://github.com/Adobe-CEP/CEP-Resources/blob/master/CEP_11.x/Documentation/CEP%2011.1%20HTML%20Extension%20Cookbook.md#debugging-unsigned-extensions)):
    -   Open `regedit`
    -   Go to the key `HKEY_CURRENT_USER/Software/Adobe/CSXS.11`
    -   Add a new entry `PlayerDebugMode` of type `string` with the value of `1`

## macOS

1.  Open `<Adobe Illustrator SDK>/sample code/LaTeX2AI/LaTeX2AI.xcodeproj` with Xcode and compile the solution.
1.  The compiled binary is located at `<Adobe Illustrator SDK>/sample code/output/`.
    This directory has to be set as the Plugin directory in Illustrator.
1.  You have to set a link to the UI folder so Illustrator can find the user interface:
    ```bash
    ln -s "<Adobe Illustrator SDK>\samplecode\LaTeX2AI\ui" "~/Library/Application Support/Adobe/CEP/extensions/com.latex2ai.latex2ai"
    ```
1.  Debugging of unsigned extensions has to be turned on ([see the Adobe CEP cookbook](https://github.com/Adobe-CEP/CEP-Resources/blob/master/CEP_11.x/Documentation/CEP%2011.1%20HTML%20Extension%20Cookbook.md#debugging-unsigned-extensions)):
    ```bash
    defaults write com.adobe.CSXS.11 PlayerDebugMode 1
    ```
