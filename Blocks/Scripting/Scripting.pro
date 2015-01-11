TEMPLATE = lib
CONFIG += plugin link_pkgconfig
QT += script

DESTDIR = ../../App/share/DSPBlocks/blocks

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += . ../../Lib
DEPENDPATH  += . ../../Lib

HEADERS +=          JS.hpp
SOURCES += main.cpp JS.cpp

PKGCONFIG += luajit
HEADERS += Lua.hpp
SOURCES += Lua.cpp
