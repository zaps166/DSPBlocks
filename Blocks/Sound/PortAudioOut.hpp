#ifndef PORTAUDIOOUT_HPP
#define PORTAUDIOOUT_HPP

#include "PortAudio.hpp"
#include "Array.hpp"

class PortAudioOut : public PortAudio
{
public:
	PortAudioOut();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	int samplesToWrite;
	Array< float > buffer, outBuffer;
};

#endif // PORTAUDIOOUT_HPP
