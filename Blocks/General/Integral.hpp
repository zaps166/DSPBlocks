#ifndef INTEGRAL_HPP
#define INTEGRAL_HPP

#include "Block.hpp"

class Integral : public Block
{
public:
	Integral();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	QVector< float > buffer[ 3 ];
};

#endif // INTEGRAL_HPP
