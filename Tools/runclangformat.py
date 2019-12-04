# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
import sys
# Prevent Python from generating .pyc files
sys.dont_write_bytecode = True

import os
import subprocess
import githelpers


clang_format ='clang-format'
style ='-style=file'
exclude_dirs = ['Build', 'tools']

def file_not_excluded(file):
    path_parts = os.path.split(file)
    for excluded in exclude_dirs:
        if excluded in path_parts[0]:
            return False
    return True

def file_was_modified(file, edited_files):
    for edited in edited_files:
        if edited and edited.lower() in file.lower():
            return True
    return False

def collect_input_files(path):
    input_files =[] 
    for root, dirs, files in os.walk(path):
        for file in files:
            if file.endswith(".cpp") or file.endswith(".h") or file.endswith(".c") or file.endswith(".hpp"):
                input_files.append(os.path.join(root, file))
    return input_files

def format_file(file):
    print("Calling clang-format on '%s' " %file)
    subprocess.Popen([clang_format, style, '-i', file])
    return

# By default, format only files under the root folder that were modified
format_all = False
arg_count = len(sys.argv)
path = githelpers.get_root()
for i in range(1, arg_count):
    if sys.argv[i] == "-all":
        format_all = True
    elif sys.argv[i] == "/?":
        print("Usage: runclangformat [path] [-all]")
        print("       Run clang format on all modified files (added, copied, modified, renamed) in Git root directory")
        print("       [path] Run on files under specific directory (and below)")
        print("       [-all] Run on all files (don't have to be modified)")
        quit()
    else:
        path = sys.argv[i]
input_files = collect_input_files(path)
edited_files = githelpers.get_edited_files(True)
edited_files = [x.replace('/', '\\') for x in edited_files] # So they match the format from collect_input_files
for file in input_files:
    # Only format IF: the file is not on the exclude list AND (the user chose to format all OR the file was modified)
    if file_not_excluded(file) == True and (format_all or file_was_modified(file, edited_files) == True):
        format_file(file)
