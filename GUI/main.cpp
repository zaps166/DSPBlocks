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

	QIcon icon;
	icon.addFile( ":/DSPBlocks-16x16" );
	icon.addFile( ":/DSPBlocks-32x32" );
	icon.addFile( ":/DSPBlocks-48x48" );
	app.setWindowIcon( icon );

	QSettings settings( QSettings::IniFormat, QSettings::UserScope, "DSPBlocks" );
	new MainWindow( settings );

	return app.exec();
}
