# Under Windows launch script start_qgit.bat needs the
# value GIT_EXEC_DIR to be set to the git bin directory
GIT_EXEC_DIR = "$$(ProgramFiles)\\Git\\bin"

# Under Windows uncomment following line to enable console messages
#CONFIG += ENABLE_CONSOLE_MSG

# check for Qt >= 5.1
CUR_QT = $$[QT_VERSION]

# WARNING greaterThan is an undocumented function
!greaterThan(CUR_QT, 5.1) {
        error("Sorry I need Qt 5.1 or later, you seem to have Qt $$CUR_QT instead")
}

# check for g++ compiler
contains(QMAKE_CC,.*g\\+\\+.*) {
        CONFIG += HAVE_GCC
}
contains(QMAKE_CC,.*gcc.*) {
        CONFIG += HAVE_GCC
}

include(../src/src.pro)

# General stuff
TEMPLATE = app
QT += widgets

#main application source file
SOURCES += $$PWD/qgit.cpp

# Platform dependent stuff
win32 {
    TARGET = qgit
    target.path = $$GIT_EXEC_DIR
    CONFIG += windows embed_manifest_exe
    RC_FILE = app_icon.rc
}

unix {
    TARGET = qgit
    #target.path = ~/bin
    target.path = /usr/bin
    CONFIG += x11
}

macx {
    TARGET = qgit
    target.path = ~/bin
    #CONFIG += x86 ppc
    RC_FILE = $$PWD/../src/resources/qgit.icns
}

HAVE_GCC {
        QMAKE_CXXFLAGS_RELEASE += -s -O2 -Wno-non-virtual-dtor -Wno-long-long -pedantic -Wconversion
        QMAKE_CXXFLAGS_DEBUG += -g3 -ggdb -O0 -Wno-non-virtual-dtor -Wno-long-long -pedantic -Wconversion
}

ENABLE_CONSOLE_MSG {
        CONFIG -= windows
        CONFIG += console
}

INSTALLS += target

# Here we generate a batch called start_qgit.bat used, under Windows only,
# to start qgit with proper PATH set.
#
# NOTE: qgit must be installed in git directory, among git exe files
# for this to work. If you install with 'make install' this is already
# done for you.
#
# Remember to set proper GIT_EXEC_DIR value at the beginning of this file
#
win32 {
    !exists($${GIT_EXEC_DIR}/git.exe) {
        error("I cannot found git files, please set GIT_EXEC_DIR in 'src.pro' file")
    }
    QGIT_BAT = ../start_qgit.bat
    CUR_PATH = $$system(echo %PATH%)
    LINE_1 = $$quote(set PATH=$$CUR_PATH;$$GIT_EXEC_DIR;)
    LINE_2 = $$quote(set PATH=$$CUR_PATH;)

    qgit_launcher.commands =    @echo @echo OFF > $$QGIT_BAT
    qgit_launcher.commands += && @echo $$LINE_1 >> $$QGIT_BAT
    qgit_launcher.commands += && @echo bin\\$$TARGET >> $$QGIT_BAT
    qgit_launcher.commands += && @echo $$LINE_2 >> $$QGIT_BAT

    QMAKE_EXTRA_TARGETS += qgit_launcher
    PRE_TARGETDEPS += qgit_launcher
}

DISTFILES += $$PWD/src/app_icon.rc $$PWD/src/helpgen.sh $$PWD/src/resources/* $$PWD/src/rc.vcproj $$PWD/src/todo.txt
DISTFILES += $$PWD/src/../COPYING $$PWD/src/../exception_manager.txt $$PWD/src/../README $$PWD/src/../README_WIN.txt
DISTFILES += $$PWD/src/../qgit_inno_setup.iss $$PWD/src/../QGit4.sln

cache()
