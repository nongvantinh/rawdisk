#!/usr/bin/env python

import os
import sys
import subprocess
import struct
from SCons.Script import Environment, Variables, Help, ARGUMENTS, EnumVariable

program_name = 'bin/disk.exe' if os.name == 'nt' else 'bin/disk.out'
root_dir = os.path.abspath('.')

opts = Variables([], ARGUMENTS)
opts.Add(EnumVariable(
    'target',
    'Compilation target',
    'debug',
    allowed_values=('debug', 'release'),
    ignorecase=2
))

env = Environment()
opts.Update(env)
Help(opts.GenerateHelpText(env))

if env['target'] == 'debug':
    if os.name == 'nt':
        env.Append(CXXFLAGS=['/W3', '/Zi', '/Od', '/EHsc'])
        env.Append(CCFLAGS=['/W3', '/Zi', '/Od', '/EHsc'])
        env.Append(CPPDEFINES=['_UNICODE', 'UNICODE'])
    else:
        env.Append(CXXFLAGS=['-g', '-O0', '-Wall', '-Wextra', '-fPIC'])
        env.Append(CCFLAGS=['-g', '-O0', '-Wall', '-Wextra', '-fPIC'])
        env.Append(CPPDEFINES=['_UNICODE', 'UNICODE'])
elif env['target'] == 'release':
    if os.name == 'nt':
        env.Append(CXXFLAGS=['/W4', '/O2'])
        env.Append(CCFLAGS=['/W4', '/O2'])
        env.Append(LINKFLAGS=['/LTCG'])
        env.Append(CPPDEFINES=['_UNICODE', 'UNICODE'])
    else:
        env.Append(CXXFLAGS=['-O2', '-Wall', '-Wextra', '-flto'])
        env.Append(CCFLAGS=['-O2', '-Wall', '-Wextra', '-flto'])
        env.Append(LINKFLAGS=['-flto'])
        env.Append(CPPDEFINES=['_UNICODE', 'UNICODE'])

env['CXXFLAGS'] = ['-std=c++17'] if os.name != 'nt' else ['/std:c++17']
env['LIBPATH'] = ['lib']

if os.name == 'nt':
    env.Append(LIBS=['kernel32', 'user32', 'gdi32', 'winspool', 'comdlg32', 'advapi32', 'shell32', 'ole32', 'oleaut32', 'uuid', 'odbc32', 'odbccp32'])

SOURCE_EXTENSION = '*.cpp'
sources = [env.Glob(SOURCE_EXTENSION)]
root_directories = ['.']

env['CPPPATH'] = root_directories
for root_dir in root_directories:
    pattern = os.path.join(root_dir, '**', SOURCE_EXTENSION)
    sources += env.Glob(pattern)

print(sources)
env.Program(target=program_name, source=sources)
