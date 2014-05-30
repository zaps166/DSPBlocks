#include "Muxer.hpp"
#include "Array.hpp"

#include "Settings.hpp"

Muxer::Muxer() :
	Block( "Multiplexer", "Przekazuje sygnał z wybranego wejścia na wyjście", 3, 1, PROCESSING )
{}

bool Muxer::start()
{
	settings->setRunMode( true );
	buffer.fill( 0.0f/0.0f, inputsCount() );
	return true;
}
void Muxer::setSample( int input, float sample )
{
	buffer[ input ] = sample;
}
void Muxer::exec( Array< Sample > &samples )
{
	if ( buffer[ 0 ] == buffer[ 0 ] ) //not NaN
		samples += ( Sample ){ getTarget( 0 ), buffer[ ( ( quint32 )buffer[ 0 ] % ( inputsCount() - 1 ) ) + 1 ] };
	else
		samples += ( Sample ){ getTarget( 0 ), 0.0f/0.0f };
}
void Muxer::stop()
{
	settings->setRunMode( false );
	buffer.clear();
}

Block *Muxer::createInstance()
{
	Muxer *block = new Muxer;
	block->settings = new Settings( *block, true, 2, maxIO, false, 1, 1 );
	return block;
}
