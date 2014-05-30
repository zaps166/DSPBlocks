TEMPLATE = lib
CONFIG += plugin
QT += script

DESTDIR = ../../App/share/MusicBlocks/blocks

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OBJECTS_DIR = build/obj
MOC_DIR = build/moc

INCLUDEPATH += ../../Lib
DEPENDPATH  += ../../Lib

HEADERS +=          Const.hpp Multiplier.hpp RandomGen.hpp Share.hpp StdOut.hpp Summation.hpp Sine.hpp Delay.hpp Gain.hpp Scope.hpp FIR.hpp VarDelay.hpp Math.hpp RMS.hpp FileWriter.hpp FileReader.hpp Clip.hpp Integral.hpp Differential.hpp IIR.hpp Counter.hpp Muxer.hpp Quantizer.hpp PWM.hpp JS.hpp
SOURCES += main.cpp Const.cpp Multiplier.cpp RandomGen.cpp Share.cpp StdOut.cpp Summation.cpp Sine.cpp Delay.cpp Gain.cpp Scope.cpp FIR.cpp VarDelay.cpp Math.cpp RMS.cpp FileWriter.cpp FileReader.cpp Clip.cpp Integral.cpp Differential.cpp IIR.cpp Counter.cpp Muxer.cpp Quantizer.cpp PWM.cpp JS.cpp

win32: {
	QMAKE_LIBDIR += ../../App
	LIBS += -lMusicBlocks
}
