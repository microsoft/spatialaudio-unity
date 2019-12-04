# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
import subprocess
import oshelpers
import os

git = 'git'
empty_sha1 = '4b825dc642cb6eb9a060e54bf8d69288fbee4904'

# Returns the root dir for the Git repo
def get_root():
    rev_parse = subprocess.Popen([git, 'rev-parse', '--show-toplevel'],
        stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    return oshelpers.fixpath(rev_parse.stdout.read().decode().strip())

# Returns the current Git HEAD
def get_head():
    rev_parse = subprocess.Popen([git, 'rev-parse', '--verify', 'HEAD'],
        stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    rev_parse.communicate()
    if rev_parse.returncode:
        return empty_sha1
    else:
        return 'HEAD'

# Returns current commit hash
def get_commit_hash():
    git_process = subprocess.Popen("git rev-parse HEAD", stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    stdout, stderr = git_process.communicate()
    return stdout.decode("utf-8")

# Returns the list of edited files
def get_edited_files(inplace):
    head = get_head()
    git_args = [git, 'diff-index']
    if not inplace:
        git_args.append('--cached')
    git_args.extend(['--diff-filter=ACMR', '--name-only', head])
    diff_index = subprocess.Popen(git_args, stdout = subprocess.PIPE)
    diff_index_ret = diff_index.stdout.read().strip()
    diff_index_ret = diff_index_ret.decode()
    return diff_index_ret.split('\n')

def get_files_under_source_control(dir):
    head = get_head()
    git_args = [git, 'ls-tree', '-r', '--name-only', head]
    diff_index = subprocess.Popen(git_args, stdout = subprocess.PIPE, cwd = dir)
    diff_index_ret = diff_index.stdout.read().strip()
    diff_index_ret = diff_index_ret.decode()
    files = diff_index_ret.split('\n')
    return files
