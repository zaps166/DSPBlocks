#include "Multiplier.hpp"
#include "Settings.hpp"
#include "Array.hpp"

Multiplier::Multiplier() :
	Block( "Multiplier", "Mnoży wejścia", 2, 1, PROCESSING )
{}

bool Multiplier::start()
{
	settings->setRunMode( true );
	buffer.reset( new float[ inputsCount() ]() );
	return true;
}
void Multiplier::setSample( int input, float sample )
{
	buffer[ input ] = sample;
}
void Multiplier::exec( Array< Sample > &samples )
{
	float output = buffer[ 0 ];
	for ( int i = 1 ; i < inputsCount() ; ++i )
		output *= buffer[ i ];
	samples += ( Sample ){ getTarget( 0 ), output };
}
void Multiplier::stop()
{
	settings->setRunMode( false );
	buffer.reset();
}

Block *Multiplier::createInstance()
{
	Multiplier *block = new Multiplier;
	block->settings = new Settings( *block, true, 2, maxIO, false, 0, 0 );
	return block;
}
