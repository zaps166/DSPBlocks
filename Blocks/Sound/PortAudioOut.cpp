#include "PortAudioOut.hpp"
#include "Global.hpp"

PortAudioOut::PortAudioOut() :
	PortAudio( "PortAudio Output", "Wyjście dźwięku PortAudio", 1, 0, SINK )
{}

bool PortAudioOut::start()
{
	samplesToWrite = Global::getBufferSize();
	PaStreamParameters streamParams = { getDeviceIndex( devName ), inputsCount(), paFloat32, Global::getPeriod()*4, NULL };
	if ( Pa_OpenStream( &stream, NULL, &streamParams, Global::getSampleRate(), 0, 0, NULL, NULL ) == paNoError )
	{
		err = false;
		settings->setRunMode( true );
		buffer.alloc( inputsCount(), true );
		outBuffer.alloc( samplesToWrite * inputsCount() );
		Pa_StartStream( stream );
		return true;
	}
	return false;
}
void PortAudioOut::setSample( int input, float sample )
{
	buffer[ input ] = sample;
}
void PortAudioOut::exec( Array< Sample > & )
{
	if ( err )
		return;
	outBuffer += buffer;
	if ( outBuffer.count() == samplesToWrite * inputsCount() )
	{
		PaError err = Pa_WriteStream( stream, outBuffer.data(), samplesToWrite );
		if ( err != paOutputUnderflowed ) //ok?
			err = true;
		else
		{
			//obsługa
		}
		outBuffer.clear();
	}
}
void PortAudioOut::stop()
{
	if ( stream )
	{
		Pa_AbortStream( stream );
		Pa_CloseStream( stream );
		stream = NULL;
		outBuffer.free();
		buffer.free();
		settings->setRunMode( false );
	}
}

Block *PortAudioOut::createInstance()
{
	PortAudioOut *block = new PortAudioOut;
	block->settings = new Settings( *block, true, 1, 8, false, 0, 0, false, new PortAudioUI( *block ) );
	return block;
}
