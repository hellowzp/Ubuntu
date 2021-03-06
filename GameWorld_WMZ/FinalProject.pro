#-------------------------------------------------
#
# Project created by QtCreator 2013-11-07T19:23:24
#
#-------------------------------------------------

QT       += core gui
CONFIG += console
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FinalProject
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++11
QT += network
mac:CONFIG += c++11

mac:QMAKE_CXXFLAGS = -mmacosx-version-min=10.7 -std=gnu0x -stdlib=libc+


SOURCES += main.cpp\
        mainwindow.cpp \
    hero.cpp \
    worldview.cpp \
    worldviewanalyzer.cpp \
    worldbase.cpp \
    worldpathfinder.cpp \
    opponent.cpp \
    heromagic.cpp \
    statusbar.cpp \
    roomframe.cpp \
    newroomdialog.cpp \
    gamehoster.cpp \
    chooseroomdialog.cpp \
    gamehostdialog.cpp \
    gameclient.cpp \
    playerframe.cpp \
    heroshadow.cpp \
    world.cpp \
    opponentshadow.cpp \
    heroreal.cpp \
    opponentreal.cpp \
    heroframe.cpp \
    assetsframe.cpp \
    gift.cpp \
    assetLabel.cpp

HEADERS  += mainwindow.h \
    world.h \
    hero.h \
    worldview.h \
    worldviewanalyzer.h \
    worldbase.h \
    worldpathfinder.h \
    opponent.h \
    heromagic.h \
    statusbar.h \
    roomframe.h \
    newroomdialog.h \
    gamehoster.h \
    chooseroomdialog.h \
    gamehostdialog.h \
    gameclient.h \
    playerframe.h \
    heroshadow.h \
    opponentshadow.h \
    heroreal.h \
    opponentreal.h \
    heroframe.h \
    assetsframe.h \
    gift.h \
    assetlabel.h

FORMS    += mainwindow.ui

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

OTHER_FILES += \
    ../../../../Pictures/Screenshot from 2013-10-10 16:57:03.png \
    ../../../../Pictures/Screenshot from 2013-10-10 16:57:03.png

RESOURCES += \
    example.qrc



INCLUDEPATH += $$PWD/../world_Linux64
DEPENDPATH += $$PWD/../world_Linux64
