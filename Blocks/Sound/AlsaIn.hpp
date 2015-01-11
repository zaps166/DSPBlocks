#ifndef ALSAIN_HPP
#define ALSAIN_HPP

#include "Alsa.hpp"

class AlsaIn : public Alsa
{
public:
	AlsaIn();

	bool start();
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	int samplesToRead;
	QScopedArrayPointer< qint32 > outBuffer;
	int outBufferPos, outBufferSize;
};

#endif // ALSAIN_HPP
