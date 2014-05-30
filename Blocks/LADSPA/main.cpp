extern const char groupName[] = "LADSPA";

#include "LADSPA.hpp"

#include <QCoreApplication>
#include <QLibrary>
#include <QDebug>
#include <QDir>

extern "C" QList< Block * > createBlocks()
{
	QStringList possibleDirs = QStringList() << qApp->property( "share" ).toString() + "/ladspa" << QDir::cleanPath( getenv( "LADSPA_PATH" ) );
#ifdef Q_OS_UNIX
	possibleDirs << ( sizeof( void * ) == 8 ? "/usr/lib64/ladspa" : "/usr/lib/ladspa" );
#endif

	QString ladspa_path;
	foreach ( QString dir, possibleDirs )
		if ( QFileInfo( dir ).isDir() )
		{
			ladspa_path = dir;
			break;
		}

	QList< Block * > ladspaBlocks;
	if ( !ladspa_path.isEmpty() )
	{
		QStringList libFilter;
#ifndef Q_OS_WIN
	#ifndef Q_OS_MAC
		libFilter << "*.so";
	#else
		libFilter << "*.dylib";
	#endif
#else
		libFilter << "*.dll";
#endif
		foreach ( QString fName, QDir( ladspa_path ).entryList( libFilter ) )
		{
			QLibrary lib( ladspa_path + "/" + fName );
			if ( lib.load() )
			{
				LADSPA_Descriptor_Function ladspa_descriptor = ( LADSPA_Descriptor_Function )lib.resolve( "ladspa_descriptor" );
				if ( !ladspa_descriptor )
					lib.unload();
				else for ( unsigned long i = 0 ;; ++i )
				{
					const LADSPA_Descriptor *ld = ladspa_descriptor( i );
					if ( !ld )
						break;
					int inputs = 0, outputs = 0;
					for ( unsigned long p = 0 ; p < ld->PortCount ; ++p )
						if ( LADSPA_IS_PORT_AUDIO( ld->PortDescriptors[ p ] ) )
						{
							if ( LADSPA_IS_PORT_INPUT( ld->PortDescriptors[ p ] ) )
								++inputs;
							else if ( LADSPA_IS_PORT_OUTPUT( ld->PortDescriptors[ p ] ) )
								++outputs;
						}
					if ( inputs > 0 && outputs > 0 )
						ladspaBlocks << new LADSPA( ld->Name, inputs, outputs, *ld );
				}
			}
		}
	}
	return ladspaBlocks;
}
