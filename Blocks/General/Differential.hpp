#ifndef DIFFERENTIAL_HPP
#define DIFFERENTIAL_HPP

#include "Block.hpp"

class Differential : public Block
{
public:
	Differential();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	QVector< float > buffer[ 2 ];
};

#endif // DIFFERENTIAL_HPP
