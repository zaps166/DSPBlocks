#ifndef RANDOMGEN_HPP
#define RANDOMGEN_HPP

#include "Block.hpp"

class RandomGen : public Block
{
public:
	RandomGen();

	bool start();
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
};

#endif // RANDOMGEN_HPP
