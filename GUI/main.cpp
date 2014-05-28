#include "MainWindow.hpp"

#include <QApplication>
#if QT_VERSION < 0x050000
	#include <QTextCodec>
#endif

int main( int argc, char *argv[] )
{
#if QT_VERSION < 0x050000
	QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "UTF-8" ) );
#endif
	QApplication app( argc, argv );
	new MainWindow;
	return app.exec();
}
