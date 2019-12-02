# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
import os
import subprocess

# Join with OS specific separators
def fixpath(*paths):
    return os.path.normpath(os.path.join(*paths))