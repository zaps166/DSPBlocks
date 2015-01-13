TEMPLATE = lib
CONFIG += plugin link_pkgconfig

DESTDIR = ../../App/share/DSPBlocks/blocks

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += . ../../Lib
DEPENDPATH  += . ../../Lib

HEADERS +=
SOURCES += main.cpp

#HEADERS += AtLPT.hpp AtLPT_Out.hpp AtLPT_In.hpp
#SOURCES += AtLPT.cpp AtLPT_Out.cpp AtLPT_In.cpp
#DEFINES += USE_ATLPT

packagesExist(comedilib) {
	PKGCONFIG += comedilib
	HEADERS += ComediOut.hpp ComediIn.hpp
	SOURCES += ComediOut.cpp ComediIn.cpp
	DEFINES += USE_COMEDI
}
