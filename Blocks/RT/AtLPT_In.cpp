#include "AtLPT_In.hpp"
#include "Settings.hpp"
#include "Array.hpp"

#include <sys/io.h>

static inline quint16 recvSample( quint8 chn )
{
	outb( ( chn << 7 ) | 0x40, EPP_DATA );
	return ( ( inb( EPP_DATA ) & 0x1F ) << 5 ) | ( inb( EPP_DATA ) >> 3 );
}

/**/

AtLPT_In::AtLPT_In() :
	AtLPT( "AtLPT input", "Wejście sygnału dla urządzenia AtLPT", 0, 1, SOURCE ),
	isOpen( false )
{}

bool AtLPT_In::start()
{
	if ( openDevice() )
	{
		settings->setRunMode( true );
		return ( isOpen = true );
	}
	return false;
}
void AtLPT_In::exec( Array< Block::Sample > &samples )
{
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), ( ( qint16 )recvSample( i ) - inputOffset[ i ] ) / ioScale[ range ] };
}
void AtLPT_In::stop()
{
	if ( isOpen )
	{
		settings->setRunMode( false );
		closeDevice();
		isOpen = false;
	}
}

Block *AtLPT_In::createInstance()
{
	AtLPT_In *block = new AtLPT_In;
	block->settings = new Settings( *block, false, 0, 0, true, 1, 2 );
	return block;
}
