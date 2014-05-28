#include "AlsaIn.hpp"
#include "Array.hpp"

AlsaIn::AlsaIn() :
	Alsa( "ALSA input", "Wejście dźwięku ALSA", 0, 1, SOURCE )
{}

bool AlsaIn::start()
{
	samplesToRead = getBufferSize();
	if ( !snd_pcm_open( &snd, devName.toLocal8Bit(), SND_PCM_STREAM_CAPTURE, 0 ) && !snd_pcm_set_params( snd, SND_PCM_FORMAT_S32, SND_PCM_ACCESS_RW_INTERLEAVED, outputsCount(), getSampleRate(), true, Block::getPeriod() * 4000000 ) )
	{
		settings->setRunMode( true );
		outBufferSize = outBufferPos = 0;
		outBuffer.resize( samplesToRead * outputsCount() );
		err = false;
		return true;
	}
	return false;
}
void AlsaIn::exec( Array< Sample > &samples )
{
	if ( !err && !outBufferSize )
	{
		if ( snd_pcm_state( snd ) == SND_PCM_STATE_XRUN && snd_pcm_prepare( snd ) )
		{
			err = true;
			outBufferSize = 0;
		}
		else
		{
			int ret = snd_pcm_readi( snd, outBuffer.data(), samplesToRead );
			if ( ret > 0 )
			{
				outBufferSize = ret * outputsCount();
//				qDebug() << ret;
			}
			else
			{
				if ( ret != -EAGAIN && ret != -EPIPE )
					err = true;
				outBufferSize = 0;
			}
			outBufferPos = 0;
		}
	}
	if ( !err && outBufferSize )
	{
		for ( int i = 0 ; i < outputsCount() ; ++i )
			samples += ( Sample ){ getTarget( i ), outBuffer[ outBufferPos + i ] / 2147483648.0f };
		outBufferPos += outputsCount();
		outBufferSize -= outputsCount();
	}
	else for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), 0.0f };
}
void AlsaIn::stop()
{
	if ( snd )
	{
		snd_pcm_drop( snd );
		snd_pcm_close( snd );
		snd = NULL;
		outBuffer.clear();
		settings->setRunMode( false );
	}
}

Block *AlsaIn::createInstance()
{
	AlsaIn *block = new AlsaIn;
	block->settings = new Settings( *block, false, 0, 0, true, 1, 8, false, new AlsaUI( *block ) );
	return block;
}
