#include "MainWindow.hpp"

#include <QApplication>
#include <QSettings>
#if QT_VERSION < 0x050000
	#include <QTextCodec>
#endif

int main( int argc, char *argv[] )
{
#if QT_VERSION < 0x050000
	QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "UTF-8" ) );
#endif
	QApplication app( argc, argv );
	app.setQuitOnLastWindowClosed( false );
	QSettings settings( QSettings::IniFormat, QSettings::UserScope, "DSPBlocks" );
	new MainWindow( settings );
	return app.exec();
}
