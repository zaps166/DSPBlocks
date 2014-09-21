TEMPLATE = lib
CONFIG += plugin

DESTDIR = ../../App/share/MusicBlocks/additions

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += . ../../Lib
DEPENDPATH  += . ../../Lib

HEADERS +=          FIR_Designer.hpp
SOURCES += main.cpp FIR_Designer.cpp

win32: {
	QMAKE_LIBDIR += ../../App
	LIBS += -lMusicBlocks
}

FORMS += \
    FIR_Designer.ui
