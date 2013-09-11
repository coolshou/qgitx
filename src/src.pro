CONFIG += qt warn_on exceptions debug_and_release c++11
QMAKE_CXXFLAGS +=-std=c++11
INCLUDEPATH += $$PWD
MAKEFILE = qmake
RESOURCES += $$PWD/icons.qrc
LIBS += -lGrantlee_Templates

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
           $$PWD/mainimpl.h $$PWD/myprocess.h $$PWD/patchcontent.h \
           $$PWD/revdesc.h $$PWD/revsview.h $$PWD/settingsimpl.h \
           $$PWD/smartbrowse.h \
    $$PWD/filehistory.h \
    $$PWD/historyview.h \
    $$PWD/navigator/navigatorcontroller.h

SOURCES += $$PWD/cache.cpp $$PWD/commitimpl.cpp \
           $$PWD/dataloader.cpp $$PWD/domain.cpp $$PWD/exceptionmanager.cpp \
           $$PWD/git.cpp $$PWD/git_startup.cpp \
           $$PWD/lanes.cpp $$PWD/mainimpl.cpp $$PWD/myprocess.cpp $$PWD/namespace_def.cpp \
           $$PWD/patchcontent.cpp \
           $$PWD/revdesc.cpp $$PWD/revsview.cpp $$PWD/settingsimpl.cpp $$PWD/smartbrowse.cpp \
    $$PWD/filehistory.cpp \
    $$PWD/historyview.cpp \
    $$PWD/navigator/navigatorcontroller.cpp
