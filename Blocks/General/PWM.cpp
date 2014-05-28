#include "PWM.hpp"
#include "Array.hpp"

#include "Settings.hpp"

PWM::PWM() :
	Block( "PWM", "Modulacja szerokości impulsów sterowana z zewnątrz", 2, 1, PROCESSING )
{}

bool PWM::start()
{
	settings->setRunMode( true );
	freq = duty = 0.5f;
	state = false;
	num_samples = 0;
	return true;
}
void PWM::setSample( int input, float sample )
{
	switch ( input )
	{
		case 0:
			if ( sample > getSampleRate() / 2.0f )
				freq = getSampleRate() / 2.0f;
			else if ( sample < 0.0f )
				freq = 0.0f;
			else
				freq = sample;
			break;
		case 1:
			if ( sample <= 0.0f )
				duty = 0.0f;
			else if ( sample >= 1.0f )
				duty = 1.0f;
			else
				duty = sample;
			break;
	}
}
void PWM::exec( Array< Sample > &samples )
{
	quint32 period = round( getSampleRate() / freq );
	quint32 hi_samples = round( period * duty );
	quint32 lo_samples = period - hi_samples;

	if ( !state && num_samples >= lo_samples )
	{
		num_samples = 0;
		if ( hi_samples > 0 )
			state = true;
	}
	if ( state && num_samples >= hi_samples )
	{
		num_samples = 0;
		if ( lo_samples > 0 )
			state = false;
	}
	++num_samples;

	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), ( float )state };
}
void PWM::stop()
{
	settings->setRunMode( false );
}

Block *PWM::createInstance()
{
	PWM *block = new PWM;
	block->settings = new Settings( *block, false, 2, 2, true, 1, maxIO );
	return block;
}
