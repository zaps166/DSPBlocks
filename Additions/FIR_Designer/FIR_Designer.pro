TEMPLATE = lib
CONFIG += plugin

DESTDIR = ../../App/share/DSPBlocks/additions

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
UI_DIR = build/ui

INCLUDEPATH += . ../../Lib
DEPENDPATH  += . ../../Lib

HEADERS +=          FIR_Designer.hpp
SOURCES += main.cpp FIR_Designer.cpp
FORMS   +=          FIR_Designer.ui

win32: {
	QMAKE_LIBDIR += ../../App
	LIBS += -lDSPBlocks
}
