TEMPLATE = lib
CONFIG += plugin

DESTDIR = ../../App/lib/DSPBlocks

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += . ../../Lib
DEPENDPATH  += . ../../Lib

HEADERS +=          LADSPA.hpp ladspa.h
SOURCES += main.cpp LADSPA.cpp

win32|macx {
	QMAKE_LIBDIR += ../../App
	LIBS += -lDSPBlocks
}
