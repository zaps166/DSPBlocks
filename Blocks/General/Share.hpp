#ifndef SHARE_HPP
#define SHARE_HPP

#include "Block.hpp"

class Share : public Block
{
public:
	Share();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	float output;
};

#endif // SHARE_HPP
