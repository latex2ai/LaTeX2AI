# -*- coding: utf-8 -*-
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
"""
Compile and create a release for LaTeX2AI
"""

import os
import sys
import subprocess
import platform
import shutil
from pathlib import Path
import argparse


from create_headers import get_git_sha


def get_git_tag_or_hash(repo=None):
    """
    Return the commit, if the commit has a tag, return the tag.
    """

    # Get current sha.
    git_sha = get_git_sha(repo)

    # Get all tags and their sha.
    process = subprocess.Popen(
        ["git", "show-ref", "--tags"], stdout=subprocess.PIPE, cwd=repo
    )
    out, _err = process.communicate()
    tags = out.decode("UTF-8").strip().split("\n")
    tags = [line.split(" ") for line in tags]
    for sha, tag in tags:
        if sha == git_sha:
            # This commit has a tag, return the tag.
            return git_sha, tag.split("/")[-1]
    return git_sha, git_sha[:7]


def clean_repository(repository_dir):
    """
    Check if the repository is clean.
    """

    # Check if the repository is clean.
    os.chdir(repository_dir)
    out = subprocess.check_output(
        ["git", "status", "--untracked-files=no", "--porcelain"]
    )
    if out == b"":
        return True
    else:
        return False


def build_solution_windows(repo_dir, git_identifier, *, build_type="release"):
    """Build the solution and compress the executable into a zip file"""

    # Build the solution, for this we have to set the build type environment
    # variable.
    script_dir = os.path.join(repo_dir, "scripts")
    os.chdir(script_dir)
    os.environ["L2A_build_type"] = build_type
    return_value = subprocess.call(["compile_solution.bat"])

    if return_value == 0:
        # The build passed, compress the executables.
        executable_dir = os.path.realpath(
            os.path.join(repo_dir, "..\\output", "win", "x64", build_type)
        )

        executable = os.path.join(executable_dir, "LaTeX2AI.aip")
        new_dir = os.path.join(repo_dir, "scripts", "release_files", "WIN")
        final_name = os.path.join(
            new_dir, "LaTeX2AI_" + git_identifier + "_" + build_type + ".aip"
        )
        Path(new_dir).mkdir(parents=True, exist_ok=True)
        shutil.move(executable, final_name)
    else:
        raise ValueError("Could not build solution in {}".format(repo_dir))


if __name__ == "__main__":
    # Execution part of script

    # Create the CLI parser
    parser = argparse.ArgumentParser(
        description="A script to build and package LaTeX2AI."
    )
    parser.add_argument("--debug", action="store_true", help="Build debug version")
    args = parser.parse_args()
    build_type = "release" if not args.debug else "debug"

    # Get some basic repo/git information
    repo_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
    git_sha, git_identifier = get_git_tag_or_hash(repo_dir)
    os.environ["L2A_GIT_IDENTIFIER"] = git_identifier

    # Check if this is a clean repository
    is_clean_repo = clean_repository(repo_dir)
    if not is_clean_repo:
        if input("Repository is not clean. Do You Want To Continue? ").lower() != "y":
            sys.exit(1)

    # Build the release version
    if platform.system() == "Windows":
        build_solution_windows(repo_dir, git_identifier, build_type=build_type)

        # Everything else is done on macOS
        sys.exit(0)
    else:
        # Build the plugin
        os.environ["L2A_BUILD_TYPE"] = build_type
        subprocess.call(os.path.join(repo_dir, "scripts/compile_mac.sh"))
        mac_release_dir = os.path.join(
            repo_dir,
            "scripts/release_files/macOS",
            f"LaTeX2AI_{git_identifier}_{build_type}.aip",
        )

        # Sign the UI folder
        subprocess.call(
            [os.path.join(repo_dir, "scripts/ui_signing/sign_ui_folder.sh")],
            # stdout=open(os.devnull, "wb"),
        )
        ui_release_dir = os.path.join(
            repo_dir,
            "scripts/release_files/",
            f"com.latex2ai.latex2ai.{git_identifier}",
        )

    # Check if the Windows binaries exist
    windows_release_file = os.path.join(
        repo_dir,
        "scripts/release_files/WIN",
        f"LaTeX2AI_{git_identifier}_{build_type}.aip",
    )
    if not os.path.isfile(windows_release_file):
        print(f'Could not find matching windows release file "{windows_release_file}"')
        sys.exit(1)

    # Create the combined release zip
    release_zip_temp_dir = os.path.join(repo_dir, "scripts/release_zip_temp/")
    if os.path.exists(release_zip_temp_dir):
        shutil.rmtree(release_zip_temp_dir)
    os.mkdir(release_zip_temp_dir)
    os.mkdir(os.path.join(release_zip_temp_dir, "WIN"))
    os.mkdir(os.path.join(release_zip_temp_dir, "macOS"))

    shutil.copyfile(
        os.path.join(repo_dir, "scripts", "release_zip_readme.md"),
        os.path.join(release_zip_temp_dir, "README.md"),
    )
    shutil.copyfile(
        windows_release_file,
        os.path.join(release_zip_temp_dir, "WIN", "LaTeX2AI.aip"),
    )
    shutil.copytree(
        mac_release_dir,
        os.path.join(release_zip_temp_dir, "macOS", "LaTeX2AI.aip"),
    )
    shutil.copytree(
        ui_release_dir,
        os.path.join(release_zip_temp_dir, "com.latex2ai.latex2ai"),
    )

    Path(os.path.join(repo_dir, "scripts/release_zip/")).mkdir(exist_ok=True)
    final_zip_name = f"../release_zip/LaTeX2AI_{git_identifier}_{build_type}.zip"
    subprocess.call(
        ["zip", "-r", final_zip_name, "."],
        cwd=release_zip_temp_dir,
    )
    print("Created the final zip file")
