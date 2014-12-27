#include "PortAudioIn.hpp"
#include "Global.hpp"
#include "Array.hpp"

#include <QDebug>

PortAudioIn::PortAudioIn() :
	PortAudio( "PortAudio Input", "Wejście dźwięku PortAudio", 0, 1, SOURCE )
{}

bool PortAudioIn::start()
{
	samplesToRead = Global::getBufferSize();
	PaStreamParameters streamParams = { getDeviceIndex( devName ), outputsCount(), paFloat32, Global::getPeriod()*4, NULL };
	if ( Pa_OpenStream( &stream, &streamParams, NULL, Global::getSampleRate(), 0, 0, NULL, NULL ) == paNoError )
	{
		err = false;
		settings->setRunMode( true );
		outBufferSize = outBufferPos = 0;
		outBuffer.resize( samplesToRead * outputsCount() );
		Pa_StartStream( stream );
		return true;
	}
	return false;
}
void PortAudioIn::exec( Array< Sample > &samples )
{
	if ( !err && !outBufferSize )
	{
		PaError err = Pa_ReadStream( stream, outBuffer.data(), samplesToRead );
		if ( err == paNoError )
			outBufferSize = samplesToRead * outputsCount();
		else if ( err == paInputOverflowed )
		{
//			qDebug() << "E2";
			//obsługa
		}
		else
		{
			err = true;
			outBufferSize = 0;
//			qDebug() << "E";
		}
		outBufferPos = 0;
	}
	if ( !err && outBufferSize )
	{
		for ( int i = 0 ; i < outputsCount() ; ++i )
			samples += ( Sample ){ getTarget( i ), outBuffer[ outBufferPos + i ] };
		outBufferPos += outputsCount();
		outBufferSize -= outputsCount();
	}
	else for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), 0.0f };
}
void PortAudioIn::stop()
{
	if ( stream )
	{
		Pa_AbortStream( stream );
		Pa_CloseStream( stream );
		stream = NULL;
		settings->setRunMode( false );
	}
}

Block *PortAudioIn::createInstance()
{
	PortAudioIn *block = new PortAudioIn;
	block->settings = new Settings( *block, false, 0, 0, true, 1, 8, false, new PortAudioUI( *block ) );
	return block;
}
