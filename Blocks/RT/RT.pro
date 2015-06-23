TEMPLATE = lib
CONFIG += plugin link_pkgconfig

DESTDIR = ../../App/lib/DSPBlocks

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += . ../../Lib
DEPENDPATH  += . ../../Lib

SOURCES += main.cpp

#HEADERS += AtLPT_Settings.hpp AtLPT.hpp AtLPT_Out.hpp AtLPT_In.hpp
#SOURCES += AtLPT_Settings.cpp AtLPT.cpp AtLPT_Out.cpp AtLPT_In.cpp
#FORMS   += AtLPT_Settings.ui
#DEFINES += USE_ATLPT

#packagesExist(comedilib) {
#	PKGCONFIG += comedilib
#	HEADERS += ComediOut.hpp ComediIn.hpp
#	SOURCES += ComediOut.cpp ComediIn.cpp
#	DEFINES += USE_COMEDI
#}
