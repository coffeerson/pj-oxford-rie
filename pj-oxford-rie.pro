QT       += core gui network widgets

CONFIG += c++17

TARGET = pj-oxford-rie
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += $$PWD/include

HEADERS += \
    $$PWD/include/Oxford133.h \
    $$PWD/include/RecipeManager.h \
    $$PWD/include/StatusMonitor.h \
    $$PWD/include/DataLogger.h \
    $$PWD/include/HardwareDiagram.h \
    $$PWD/include/mainwindow.h

SOURCES += \
    $$PWD/src/Oxford133.cpp \
    $$PWD/src/RecipeManager.cpp \
    $$PWD/src/StatusMonitor.cpp \
    $$PWD/src/DataLogger.cpp \
    $$PWD/src/HardwareDiagram.cpp \
    $$PWD/src/mainwindow.cpp \
    $$PWD/src/main.cpp
