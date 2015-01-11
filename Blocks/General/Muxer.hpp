#ifndef MUXER_HPP
#define MUXER_HPP

#include "Block.hpp"

class Muxer : public Block
{
public:
	Muxer();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	QScopedArrayPointer< float > buffer;
};

#endif // MUXER_HPP
