project('stacktrace', 'c',
  version : '0.1.0',
  license : 'BSD-2-Clause')

# dependencies

libunwind = dependency('libunwind')

# libraries

cc = meson.get_compiler('c')
libdl = cc.find_library('dl')

# subdirectories

includes = include_directories('src')

# libstacktrace

install_headers('src/stacktrace.h')

lib_sources = [
    'src/stacktrace.c'
]
libstacktrace = shared_library('stacktrace',
                               lib_sources,
                               dependencies : [libunwind, libdl],
                               install : true)

# tests

test_sources = [
    'test/test.c'
]
testexe = executable('testexe', test_sources,
                     include_directories : includes,
                     link_with : libstacktrace)
test('stacktrace test', testexe)

# pkg-config

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : libstacktrace,
                 version : '0.1',
                 name : 'libstacktrace',
                 filebase : 'stacktrace',
                 description : 'A Library to dump stack trace.')
