greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = lib
TARGET = MusicBlocks

win32: DESTDIR = ../App
else:  DESTDIR = ../App/lib

SOURCES += Scene.cpp Block.cpp Settings.cpp
HEADERS += Scene.hpp Block.hpp Settings.hpp Array.hpp RingBuffer.hpp Functions.hpp

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
