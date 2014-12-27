TEMPLATE = lib
CONFIG += plugin

DESTDIR = ../../App/share/DSPBlocks/blocks

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += . ../../Lib
DEPENDPATH  += . ../../Lib

HEADERS +=          AtLPT.hpp AtLPT_Out.hpp AtLPT_In.hpp
SOURCES += main.cpp AtLPT.cpp AtLPT_Out.cpp AtLPT_In.cpp
