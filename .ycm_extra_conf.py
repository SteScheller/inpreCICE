"""This module provides compilation flags used by ycm to enable
semantic code completion."""

import os
import sys

DIR_OF_THIS_SCRIPT = os.path.abspath(os.path.dirname( __file__ ))

CPP_EXT = {
        '.cpp', '.cc', '.c++', '.cp', '.cxx',
        '.hpp', '.hh', '.h++', '.hp', '.hxx'}
C_EXT = {'.c', '.h'}

INCLUDE = [
        './src',
        './src/draw',
        './include',
        './lib/gl3w',
        './lib/imgui',
        './lib/nlohmann']

CPP_FLAGS = ['-std=c++14']
C_FLAGS = ['-std=c11']
COMMON_FLAGS = [
        '-fopenmp',
        '`pkg-config --cflags glfw3`',
        '-Wall',
        '-Wextra',
        '-DDEBUG',
        '-g',
        '-Og']

LIBS = [
        '-lGL',
        '`pkg-config --static --libs glfw3`',
        '-fopenmp',
        '-lboost_program_options',
        '-lboost_iostreams',
        '-lfreeimage',
        '-lprecice']

# log output for debugging
log = None
if log is not None and os.path.isfile(log):
    os.remove(log)

def FlagsForFile(filename, **kwargs):
    includes = ['-I' + os.path.join(DIR_OF_THIS_SCRIPT, item) for \
            item in INCLUDE]

    flags = COMMON_FLAGS
    if os.path.splitext(filename)[1].lower() in CPP_EXT:
        flags.extend(CPP_FLAGS)
    else:
        flags.extend(C_FLAGS)

    if log is not None:
        orig_stdout = sys.stdout
        with open(log, 'a') as f:
            sys.stdout = f
            print(filename)
            print(kwargs)
            print(kwargs['client_data'])
            for item in kwargs['client_data']:
                print(item, kwargs['client_data'][item])
            print(flags)
            print(includes)
            print()
        sys.stdout = orig_stdout

    return {'flags': flags + includes + LIBS }


