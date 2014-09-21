TEMPLATE = lib
CONFIG += plugin

DESTDIR = ../../App/share/MusicBlocks/blocks

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += . ../../Lib
DEPENDPATH  += . ../../Lib

HEADERS +=          LADSPA.hpp ladspa.h
SOURCES += main.cpp LADSPA.cpp

win32: {
	QMAKE_LIBDIR += ../../App
	LIBS += -lMusicBlocks
}
