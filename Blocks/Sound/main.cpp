#ifdef USE_ALSA
	#include "AlsaOut.hpp"
	#include "AlsaIn.hpp"
#endif
//#include "Equalizer.hpp"
#include "FFMpegIn.hpp"
#include "Image.hpp"
#ifdef USE_PORTAUDIO
	#include "PortAudioIn.hpp"
	#include "PortAudioOut.hpp"
#endif
#include "Spectrum.hpp"

extern "C"
{
	#include <libavformat/avformat.h>
}

extern "C" QList< Block * > createBlocks()
{
	static bool once;
	if ( !once )
	{
		av_register_all();
		avformat_network_init();
#ifdef USE_PORTAUDIO
		Pa_Initialize();
#endif
		once = true;
	}
	return QList< Block * >()
#ifdef USE_ALSA
		<< new AlsaIn
		<< new AlsaOut
#endif
//		<< new Equalizer
		<< new FFMpegIn
		<< new Image
#ifdef USE_PORTAUDIO
		<< new PortAudioIn
		<< new PortAudioOut
#endif
		<< new Spectrum
	;
}
