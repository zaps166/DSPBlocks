TEMPLATE = lib
CONFIG += plugin link_pkgconfig

DESTDIR = ../../App/share/DSPBlocks/blocks

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += . ../../Lib
DEPENDPATH  += . ../../Lib

HEADERS +=          Scripting.hpp
SOURCES += main.cpp Scripting.cpp

QT += script
HEADERS += JS.hpp
SOURCES += JS.cpp
DEFINES += USE_JS

PKGCONFIG += luajit
HEADERS += Lua.hpp
SOURCES += Lua.cpp
DEFINES += USE_LUA
