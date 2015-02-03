#include "AtLPT_Out.hpp"
#include "Settings.hpp"

#include <sys/io.h>

static inline void sendSample( quint16 sample, quint8 chn )
{
	outb( ( chn << 7 ) | ( ( sample >> 6 ) & 0x0F ), EPP_DATA );
	outb( sample << 2, EPP_DATA );
}

/**/

AtLPT_Out::AtLPT_Out() :
	AtLPT( "AtLPT output", "Wyjście sygnału dla urządzenia AtLPT", 1, 0, SINK ),
	isOpen( false )
{}

bool AtLPT_Out::start()
{
	if ( openDevice() )
	{
		settings->setRunMode( true );
		buffer.reset( new quint16[ inputsCount() ]() );
		return ( isOpen = true );
	}
	return false;
}
void AtLPT_Out::setSample( int input, float sample )
{
	const qint32 scaledSample = ( qint32 )( sample * ioScale[ range ] ) + outputOffset[ input ];
	if ( scaledSample < 0 )
		buffer[ input ] = 0;
	else if ( scaledSample > 1023 )
		buffer[ input ] = 1023;
	else
		buffer[ input ] = scaledSample;
}
void AtLPT_Out::exec( Array< Sample > & )
{
	for ( int i = 0 ; i < inputsCount() ; ++i )
		sendSample( buffer[ i ], i );
}
void AtLPT_Out::stop()
{
	if ( isOpen )
	{
		for ( int i = 0 ; i < inputsCount() ; ++i )
			sendSample( outputOffset[ i ], i ); //set outputs to 0
		closeDevice();
		buffer.reset();
		settings->setRunMode( false );
		isOpen = false;
	}
}

Block *AtLPT_Out::createInstance()
{
	AtLPT_Out *block = new AtLPT_Out;
	block->settings = new Settings( *block, true, 1, 2, false, 0, 0 );
	return block;
}
