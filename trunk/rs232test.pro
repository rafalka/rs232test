#-------------------------------------------------
#
# Project created by QtCreator 2012-04-25T21:35:47
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = rs232test
TEMPLATE = app

SOURCES += \
    src/main.cpp\
    src/MainWindow.cpp \
    src/strbinconv.cpp \
    src/SerialDeviceInterface.cpp \
    src/serialsetupdialog.cpp \
    src/InputHistoryList.cpp \
    src/BinaryEditor.cpp \
    src/MacrosEditDialog.cpp \
    3rdpty/qhexedit2/src/xbytearray.cpp \
    3rdpty/qhexedit2/src/qhexedit_p.cpp \
    3rdpty/qhexedit2/src/qhexedit.cpp \
    3rdpty/qhexedit2/src/commands.cpp \
    common/strutils.c \
    common/printopt.c \
    common/parseopt.c \
    common/vfile.c

HEADERS  += \
    src/MainWindow.h \
    src/strbinconv.h \
    src/SerialDeviceInterface.h \
    src/appconfig.h \
    src/serialsetupdialog.h \
    src/InputHistoryList.h \
    src/BinaryEditor.h \
    src/debug.h \
    src/cpputils.h \
    src/MacrosEditDialog.h \
    3rdpty/qhexedit2/src/xbytearray.h \
    3rdpty/qhexedit2/src/qhexedit_p.h \
    3rdpty/qhexedit2/src/qhexedit.h \
    3rdpty/qhexedit2/src/commands.h \
    common/strutils.h \
    common/printopt.h \
    common/parseopt.h \
    common/vfile.h

FORMS    += \
    ui/MainWindow.ui \
    ui/serialsetupdialog.ui \
    ui/BinaryEditor.ui \
    ui/MacrosEditDialog.ui

RESOURCES += \
    res/icons_16.qrc \
    res/buttons.qrc

INCLUDEPATH += src common 3rdpty/qhexedit2/src

#INCLUDEPATH += ../../3rdpty/boost/include
#LIBS        += -L../../3rdpty/boost/lib
#-lwsock32

include(3rdpty/qexserialport/src/qextserialport.pri)
win32 {
  DEFINES += _WIN32_WINNT=0x0501
  RC_FILE = myapp.rc
}

debug {
    CONFIG += console
#    LIBS   +=-lboost_system-mgw46-mt-1_52
}

release {
    CONFIG -= console
#    LIBS   +=-lboost_system-mgw46-mt-d-1_52
}

CONFIG += static
