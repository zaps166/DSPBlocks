QT += script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = lib
TARGET = MusicBlocks

INCLUDEPATH += .
DEPENDPATH  += .

win32: DESTDIR = ../App
else:  DESTDIR = ../App/lib

SOURCES += Scene.cpp Block.cpp Settings.cpp WindFunc.cpp GraphW.cpp DrawHelper.cpp
HEADERS += Scene.hpp Block.hpp Settings.hpp WindFunc.hpp GraphW.hpp DrawHelper.hpp Array.hpp RingBuffer.hpp Functions.hpp

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
