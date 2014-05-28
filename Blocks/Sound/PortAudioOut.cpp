#include "PortAudioOut.hpp"

PortAudioOut::PortAudioOut() :
	PortAudio( "PortAudio Output", "Wyjście dźwięku PortAudio", 1, 0, SINK )
{}

bool PortAudioOut::start()
{
	samplesToWrite = getBufferSize();
	PaStreamParameters streamParams = { getDeviceIndex( devName ), inputsCount(), paFloat32, Block::getPeriod()*4, NULL };
	if ( Pa_OpenStream( &stream, NULL, &streamParams, getSampleRate(), 0, 0, NULL, NULL ) == paNoError )
	{
		err = false;
		settings->setRunMode( true );
		buffer.resize( inputsCount() );
		inBuffer.alloc( samplesToWrite * inputsCount() );
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
	inBuffer += buffer;
	if ( inBuffer.count() == samplesToWrite * inputsCount() )
	{
		PaError err = Pa_WriteStream( stream, inBuffer.data(), samplesToWrite );
		if ( err != paOutputUnderflowed ) //ok?
			err = true;
		else
		{
			//obsługa
		}
		inBuffer.clear();
	}
}
void PortAudioOut::stop()
{
	if ( stream )
	{
		Pa_AbortStream( stream );
		Pa_CloseStream( stream );
		stream = NULL;
		inBuffer.free();
		settings->setRunMode( false );
	}
}

Block *PortAudioOut::createInstance()
{
	PortAudioOut *block = new PortAudioOut;
	block->settings = new Settings( *block, true, 1, 8, false, 0, 0, false, new PortAudioUI( *block ) );
	return block;
}
