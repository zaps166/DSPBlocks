QT += script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = lib
TARGET = DSPBlocks

INCLUDEPATH += .
DEPENDPATH  += .

win32: DESTDIR = ../App
else:  DESTDIR = ../App/lib

#linux*: LIBS += -Wl,@/usr/xenomai/lib/posix.wrappers -L/usr/xenomai/lib -lpthread_rt -lxenomai

SOURCES += Scene.cpp Block.cpp Settings.cpp WindFunc.cpp GraphW.cpp DrawHelper.cpp Global.cpp Thread.cpp
HEADERS += Scene.hpp Block.hpp Settings.hpp WindFunc.hpp GraphW.hpp DrawHelper.hpp Global.hpp Thread.hpp Array.hpp RingBuffer.hpp

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
