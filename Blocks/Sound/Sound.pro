TEMPLATE = lib
CONFIG += plugin

DESTDIR = ../../App/share/DSPBlocks

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += . ../../Lib
DEPENDPATH  += . ../../Lib

HEADERS +=          FFMpegIn.hpp Spectrum.hpp Image.hpp
SOURCES += main.cpp FFMpegIn.cpp Spectrum.cpp Image.cpp

!win32: CONFIG += link_pkgconfig
if ( win32|packagesExist(portaudio-2.0) ) {
	HEADERS += PortAudioOut.hpp PortAudioIn.hpp PortAudio.hpp
	SOURCES += PortAudioOut.cpp PortAudioIn.cpp PortAudio.cpp
	DEFINES += USE_PORTAUDIO
	win32: LIBS += -lportaudio
	else: PKGCONFIG += portaudio-2.0
}

#HEADERS += Equalizer.hpp
#SOURCES += Equalizer.cpp

DEFINES += __STDC_CONSTANT_MACROS
LIBS += -lavformat -lavcodec -lavutil -lswresample

linux*: {
	HEADERS += AlsaOut.hpp AlsaIn.hpp Alsa.hpp
	SOURCES += AlsaOut.cpp AlsaIn.cpp Alsa.cpp
	DEFINES += USE_ALSA
	LIBS += -lasound
}

win32: {
	QMAKE_LIBDIR += ../../App
	LIBS += -lDSPBlocks -lwinmm -luuid
}
