TEMPLATE = lib
CONFIG += plugin

DESTDIR = ../../App/share/DSPBlocks/blocks

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += . ../../Lib
DEPENDPATH  += . ../../Lib

HEADERS +=          Scripting.hpp CodeEdit.hpp SyntaxHighlighter.hpp
SOURCES += main.cpp Scripting.cpp CodeEdit.cpp SyntaxHighlighter.cpp

QT += script
HEADERS += JS.hpp
SOURCES += JS.cpp

!win32: CONFIG += link_pkgconfig
if ( win32|packagesExist(luajit) ) {
	HEADERS += Lua.hpp
	SOURCES += Lua.cpp
	DEFINES += USE_LUA
	win32: {
		INCLUDEPATH += luajit-2.0
		LIBS += -lluajit
	}
	else: PKGCONFIG += luajit
}

win32: {
	QMAKE_LIBDIR += ../../App
	LIBS += -lDSPBlocks
}
