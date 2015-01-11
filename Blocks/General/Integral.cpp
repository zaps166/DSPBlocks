#include "Integral.hpp"
#include "Settings.hpp"
#include "Array.hpp"

Integral::Integral() :
	Block( "Integral", "Liczy całkę z sygnału", 1, 1, PROCESSING )
{}

bool Integral::start()
{
	if ( inputsCount() != outputsCount() )
		return false;
	for ( int i = 0 ; i < 3 ; ++i )
		buffer[ i ].reset( new float[ inputsCount() ]() );
	settings->setRunMode( true );
	return true;
}
void Integral::setSample( int input, float sample )
{
	buffer[ 1 ][ input ] = sample;
}
void Integral::exec( Array< Sample > &samples )
{
	for ( int i = 0 ; i < outputsCount() ; ++i )
	{
		buffer[ 2 ][ i ] += ( buffer[ 0 ][ i ] + buffer[ 1 ][ i ] ) / 2.0f;
		samples += ( Sample ){ getTarget( i ), buffer[ 2 ][ i ] };
		buffer[ 0 ][ i ] = buffer[ 1 ][ i ];
	}
}
void Integral::stop()
{
	settings->setRunMode( false );
	for ( int i = 0 ; i < 3 ; ++i )
		buffer[ i ].reset();
}

Block *Integral::createInstance()
{
	Integral *block = new Integral;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true );
	return block;
}
