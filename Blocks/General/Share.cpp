#include "Share.hpp"
#include "Array.hpp"
#include "Settings.hpp"

Share::Share() :
	Block( "Share", "Rozdziela sygnał", 1, 2, PROCESSING )
{}

bool Share::start()
{
	output = 0.0f;
	settings->setRunMode( true );
	return true;
}
void Share::setSample( int, float sample )
{
	output = sample;
}
void Share::exec( Array< Sample > &samples )
{
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), output };
}
void Share::stop()
{
	settings->setRunMode( false );
}

Block *Share::createInstance()
{
	Share *block = new Share;
	block->settings = new Settings( *block, false, 0, 0, true, 1, maxIO );
	return block;
}
