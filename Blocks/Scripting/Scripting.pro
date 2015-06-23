TEMPLATE = lib
CONFIG += plugin

DESTDIR = ../../App/lib/DSPBlocks

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

!win32 {
	macx:QT_CONFIG -= no-pkg-config
	CONFIG += link_pkgconfig
}
if ( win32|packagesExist(luajit) ) {
	HEADERS += Lua.hpp
	SOURCES += Lua.cpp
	DEFINES += USE_LUA
	win32 {
#		INCLUDEPATH += /usr/local/include/luajit-2.0
#		QMAKE_LIBDIR += /usr/local/lib
		LIBS += -lluajit-5.1
	}
	else: PKGCONFIG += luajit
}

win32|macx {
	QMAKE_LIBDIR += ../../App
	LIBS += -lDSPBlocks
}
