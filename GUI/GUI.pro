#-------------------------------------------------
#
# Project created by QtCreator 2014-02-25T21:07:24
#
#-------------------------------------------------

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = DSPBlocks

CONFIG += console

win32: DESTDIR = ../App
else:  DESTDIR = ../App/bin

win32 {
	QMAKE_LIBDIR += ../App
	LIBS += -lwinmm
} else {
	!mac: QMAKE_LFLAGS += -Wl,-rpath=\'\$\$ORIGIN\'/../lib
	QMAKE_LIBDIR += ../App/lib
	LIBS += -lrt
}
LIBS += -lDSPBlocks

SOURCES += main.cpp MainWindow.cpp BlocksTree.cpp SchemeView.cpp SimSettings.cpp
HEADERS +=          MainWindow.hpp BlocksTree.hpp SchemeView.hpp SimSettings.hpp
FORMS   +=          MainWindow.ui                                SimSettings.ui

linux*: {
	SOURCES += RTSettings.cpp
	HEADERS += RTSettings.hpp
	FORMS   += RTSettings.ui
}

RESOURCES += Res.qrc

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
UI_DIR = build/ui

INCLUDEPATH += . ../Lib
DEPENDPATH  += . ../Lib
