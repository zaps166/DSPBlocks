#ifndef MULTIPLIER_HPP
#define MULTIPLIER_HPP

#include "Block.hpp"

class Multiplier : public Block
{
public:
	Multiplier();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	QVector< float > buffer;
};

#endif // MULTIPLIER_HPP
