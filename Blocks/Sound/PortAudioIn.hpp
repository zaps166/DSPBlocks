#ifndef PORTAUDIOIN_HPP
#define PORTAUDIOIN_HPP

#include "PortAudio.hpp"

class PortAudioIn : public PortAudio
{
public:
	PortAudioIn();

	bool start();
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	int samplesToRead;
	QVector< float > outBuffer;
	int outBufferPos, outBufferSize;
};

#endif // PORTAUDIOIN_HPP
