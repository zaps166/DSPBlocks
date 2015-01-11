#ifdef USE_ALSA
	#include "AlsaOut.hpp"
	#include "AlsaIn.hpp"
#endif
//#include "Equalizer.hpp"
#include "FFMpegIn.hpp"
#include "Image.hpp"
#include "PortAudioIn.hpp"
#include "PortAudioOut.hpp"
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
		Pa_Initialize();
		once = true;
	}
	return QList< Block * >() <<
#ifdef USE_ALSA
		new AlsaIn << new AlsaOut <<
#endif
		/*new Equalizer <<*/ new FFMpegIn << new Image << new PortAudioIn << new PortAudioOut << new Spectrum;
}
