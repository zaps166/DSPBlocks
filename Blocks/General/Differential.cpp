#include "Differential.hpp"
#include "Settings.hpp"
#include "Array.hpp"

Differential::Differential() :
	Block( "du/dt", "Liczy różniczkę z sygnału", 1, 1, PROCESSING )
{}

bool Differential::start()
{
	if ( inputsCount() != outputsCount() )
		return false;
	for ( int i = 0 ; i < 2 ; ++i )
		buffer[ i ].reset( new float[ inputsCount() ]() );
	settings->setRunMode( true );
	return true;
}
void Differential::setSample( int input, float sample )
{
	buffer[ 1 ][ input ] = sample;
}
void Differential::exec( Array< Sample > &samples )
{
	for ( int i = 0 ; i < outputsCount() ; ++i )
	{
		samples += ( Sample ){ getTarget( i ), buffer[ 1 ][ i ] - buffer[ 0 ][ i ] };
		buffer[ 0 ][ i ] = buffer[ 1 ][ i ];
	}
}
void Differential::stop()
{
	settings->setRunMode( false );
	for ( int i = 0 ; i < 2 ; ++i )
		buffer[ i ].reset();
}

Block *Differential::createInstance()
{
	Differential *block = new Differential;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true );
	return block;
}
