TEMPLATE=subdirs
SUBDIRS=src
CONFIG += debug_and_release c++11

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -stdlib=libc++
QMAKE_CXXFLAGS += -mmacosx-version-min=10.7
