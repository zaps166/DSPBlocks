#include "AlsaOut.hpp"
#include "Global.hpp"

AlsaOut::AlsaOut() :
	Alsa( "ALSA output", "Wyjście dźwięku ALSA", 1, 0, SINK )
{}

bool AlsaOut::start()
{
	samplesToWrite = Global::getBufferSize();
	if ( !snd_pcm_open( &snd, devName.toLocal8Bit(), SND_PCM_STREAM_PLAYBACK, 0 ) && !snd_pcm_set_params( snd, SND_PCM_FORMAT_S32, SND_PCM_ACCESS_RW_INTERLEAVED, inputsCount(), Global::getSampleRate(), true, Global::getPeriod() * 4000000 ) )
	{
		settings->setRunMode( true );
		buffer.alloc( inputsCount(), true );
		inBuffer.alloc( samplesToWrite * inputsCount() );
		err = false;
		return true;
	}
	return false;
}
void AlsaOut::setSample( int input, float sample )
{
	if ( sample > 1.0f )
		sample = 1.0f;
	else if ( sample < -1.0f )
		sample = -1.0f;
	else if ( sample != sample ) //NaN
		sample = 0.0f;
	buffer[ input ] = sample * 2147483647.0;
}
void AlsaOut::exec( Array< Sample > & )
{
	if ( err )
		return;
	inBuffer += buffer;
	if ( inBuffer.count() == samplesToWrite * inputsCount() )
	{
		if ( snd_pcm_state( snd ) == SND_PCM_STATE_XRUN && !snd_pcm_prepare( snd ) )
		{
//			const int silence = snd_pcm_avail( snd ) - samplesToWrite;
//			if ( silence > 0 )
//			{
//				QByteArray silenceArr( silence * inputsCount() * sizeof( qint32 ), 0 );
//				snd_pcm_writei( snd, silenceArr.data(), silence );
//			}
//			qDebug() << "XRUN";
		}
		const int ret = snd_pcm_writei( snd, inBuffer.data(), samplesToWrite );
		if ( ret < 0 && ret != -EPIPE && snd_pcm_recover( snd, ret, false ) )
			err = true;
		inBuffer.clear();
	}
}
void AlsaOut::stop()
{
	if ( snd )
	{
		snd_pcm_drop( snd );
		snd_pcm_close( snd );
		snd = NULL;
		buffer.free();
		inBuffer.free();
		settings->setRunMode( false );
	}
}

Block *AlsaOut::createInstance()
{
	AlsaOut *block = new AlsaOut;
	block->settings = new Settings( *block, true, 1, 8, false, 0, 0, false, new AlsaUI( *block ) );
	return block;
}
