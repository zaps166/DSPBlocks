#include "RandomGen.hpp"
#include "Array.hpp"
#include "Settings.hpp"

RandomGen::RandomGen() :
	Block( "Random generator", "Generuje losowe próbki", 0, 1, SOURCE )
{}

bool RandomGen::start()
{
	settings->setRunMode( true );
	return true;
}
void RandomGen::exec( Array< Sample > &samples )
{
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), qrand() / ( RAND_MAX / 2.0f ) - 1.0f };
}
void RandomGen::stop()
{
	settings->setRunMode( false );
}

Block *RandomGen::createInstance()
{
	RandomGen *block = new RandomGen;
	block->settings = new Settings( *block, false, 0, 0, true, 1, maxIO );
	return block;
}
