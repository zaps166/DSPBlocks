TEMPLATE = lib
CONFIG += plugin

DESTDIR = ../../App/share/MusicBlocks/blocks

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += ../../Lib
DEPENDPATH  += ../../Lib

HEADERS +=          FFMpegIn.hpp Spectrum.hpp Equalizer.hpp PortAudioOut.hpp PortAudioIn.hpp PortAudio.hpp Image.hpp
SOURCES += main.cpp FFMpegIn.cpp Spectrum.cpp Equalizer.cpp PortAudioOut.cpp PortAudioIn.cpp PortAudio.cpp Image.cpp

DEFINES += __STDC_CONSTANT_MACROS
LIBS += -lavformat -lavcodec -lavutil -lswresample -lportaudio

linux*: {
	HEADERS += AlsaOut.hpp AlsaIn.hpp Alsa.hpp
	SOURCES += AlsaOut.cpp AlsaIn.cpp Alsa.cpp
	DEFINES += USE_ALSA
	LIBS += -lasound
}

win32: {
	QMAKE_LIBDIR += ../../App
	LIBS += -lMusicBlocks -lwinmm -luuid
}
