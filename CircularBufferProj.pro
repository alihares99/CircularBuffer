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

#DEFINES += QCUSTOMPLOT_USE_OPENGL

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

allFilesFolders = $$files(src/*, true))
folders = src Libs
for (item, allFilesFolders): contains(item, [^.]*) : folders += $$item
INCLUDEPATH += $$folders
INCLUDEPATH += 'Parts/H_Tracker/src/'

win32: INCLUDEPATH += 'C:\dev\eigen'
win32: INCLUDEPATH += 'C:\dev\GeographicLib\include'
unix: LIBS += /usr/local/lib/libGeographic.so

CONFIG(debug, debug|release ) {
    # debug
    win32: LIBS += -L'C:\dev\GeographicLib\lib' -lGeographic_d
} else {
    # release
    win32: LIBS += -L'C:\dev\GeographicLib\lib' -lGeographic
}
