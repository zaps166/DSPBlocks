#include "AtLPT_Out.hpp"
#include "Settings.hpp"

#include <sys/io.h>

static inline void sendSample( qint16 sample, quint8 chn )
{
	sample += 0x1FF;
	outb( ( chn << 7 ) | ( ( sample >> 6 ) & 0x0F ), EPP_DATA );
	outb( sample << 2, EPP_DATA );
}

/**/

AtLPT_Out::AtLPT_Out() :
	AtLPT( "AtLPT output", "Wyjście sygnału dla urządzenia AtLPT", 1, 0, SINK )
{}

bool AtLPT_Out::start()
{
	if ( openDevice() )
	{
		settings->setRunMode( true );
		buffer.resize( inputsCount() );
		return true;
	}
	return false;
}
void AtLPT_Out::setSample( int input, float sample )
{
	if ( sample > 1.0f )
		sample = 1.0f;
	else if ( sample < -1.0f )
		sample = -1.0f;
	else if ( sample != sample ) //NaN
		sample = 0.0f;
	buffer[ input ] = sample * 511.0;
}
void AtLPT_Out::exec( Array< Sample > & )
{
	for ( int i = 0 ; i < inputsCount() ; ++i )
		sendSample( buffer[ i ], i );
}
void AtLPT_Out::stop()
{
	closeDevice();
	buffer.clear();
	settings->setRunMode( false );
}

Block *AtLPT_Out::createInstance()
{
	AtLPT_Out *block = new AtLPT_Out;
	block->settings = new Settings( *block, true, 1, 2, false, 0, 0 );
	return block;
}
