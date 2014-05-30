#ifndef ALSAOUT_HPP
#define ALSAOUT_HPP

#include "Alsa.hpp"
#include "Array.hpp"

class AlsaOut : public Alsa
{
public:
	AlsaOut();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	int samplesToWrite;
	QVector< qint32 > buffer;
	Array< qint32 > inBuffer;
};

#endif // ALSAOUT_HPP
