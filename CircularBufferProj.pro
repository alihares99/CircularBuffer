QT += core network gui widgets printsupport

TEMPLATE = app
TARGET = CircularBufferProj
CONFIG += c++17 precompile_header

OBJECTS_DIR = release
MOC_DIR = release

PRECOMPILED_HEADER = src/PreCompiledHeader.h

SOURCES += $$files(src/*.cpp, true) $$files(Libs/*.cpp, true)

HEADERS += $$files(src/*.h, true) $$files(Libs/*.h, true) $$files(Libs/*.hpp, true)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
           NOMINMAX \
           _HAS_CXX17=1
win32:DEFINES += _GLIBCXX_TXN_SAFE_DYN="" _GLIBCXX_NOTHROW="" # For compatibility to GCC

allFilesFolders = $$files(src/*, true))
folders = src
for (item, allFilesFolders): contains(item, [^.]*) : folders += $$item
INCLUDEPATH += $$folders
