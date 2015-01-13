TEMPLATE = lib
CONFIG += plugin link_pkgconfig

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

packagesExist(luajit) {
	PKGCONFIG += luajit
	HEADERS += Lua.hpp
	SOURCES += Lua.cpp
	DEFINES += USE_LUA
}

win32: {
	QMAKE_LIBDIR += ../../App
	LIBS += -lDSPBlocks
}
