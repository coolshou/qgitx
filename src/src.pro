CONFIG += qt warn_on exceptions debug_and_release c++11
QMAKE_CXXFLAGS +=-std=c++11
INCLUDEPATH += $$PWD
MAKEFILE = qmake
RESOURCES += $$PWD/icons.qrc
LIBS += -lGrantlee_Templates
QT += webkitwidgets

# Directories
DESTDIR = $$PWD/../bin
BUILD_DIR = $$PWD/../build
UI_DIR = $$BUILD_DIR
MOC_DIR = $$BUILD_DIR
RCC_DIR = $$BUILD_DIR
OBJECTS_DIR = $$BUILD_DIR

# project files
FORMS += $$PWD/commit.ui $$PWD/help.ui \
         $$PWD/mainview.ui $$PWD/revsview.ui $$PWD/settings.ui

HEADERS += $$PWD/cache.h $$PWD/commitimpl.h $$PWD/common.h $$PWD/config.h \
           $$PWD/dataloader.h $$PWD/domain.h $$PWD/exceptionmanager.h \
           $$PWD/git.h $$PWD/help.h $$PWD/lanes.h \
           $$PWD/mainimpl.h $$PWD/myprocess.h \
           $$PWD/revdesc.h $$PWD/revsview.h $$PWD/settingsimpl.h \
    $$PWD/filehistory.h \
    $$PWD/historyview.h \
    $$PWD/navigator/navigatorcontroller.h \
    $$PWD/diff/DiffLine.h \
    $$PWD/diff/TreeDiff.h \
    $$PWD/diff/FileDiff.h \
    $$PWD/diff/Hunk.h \
    $$PWD/diff/diff.h \
    $$PWD/diff/diff_def.h \
    $$PWD/tools/tools.h \
    $$PWD/tools/optional.h \
    $$PWD/tools/maybe.h \
    $$PWD/ui/searchedit.h

SOURCES += $$PWD/cache.cpp $$PWD/commitimpl.cpp \
           $$PWD/dataloader.cpp $$PWD/domain.cpp $$PWD/exceptionmanager.cpp \
           $$PWD/git.cpp $$PWD/git_startup.cpp \
           $$PWD/lanes.cpp $$PWD/mainimpl.cpp $$PWD/myprocess.cpp $$PWD/namespace_def.cpp \
           $$PWD/revdesc.cpp $$PWD/revsview.cpp $$PWD/settingsimpl.cpp \
    $$PWD/filehistory.cpp \
    $$PWD/historyview.cpp \
    $$PWD/navigator/navigatorcontroller.cpp \
    $$PWD/diff/DiffLine.cpp \
    $$PWD/diff/TreeDiff.cpp \
    $$PWD/diff/FileDiff.cpp \
    $$PWD/diff/Hunk.cpp \
    $$PWD/diff/diff.cpp \
    $$PWD/tools/tools.cpp \
    $$PWD/tools/maybe.cpp \
    $$PWD/ui/searchedit.cpp
