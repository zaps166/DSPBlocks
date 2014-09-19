#ifndef ATLPT_IN_HPP
#define ATLPT_IN_HPP

#include "AtLPT.hpp"

class AtLPT_In : public AtLPT
{
public:
	AtLPT_In();

	bool start();
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
};

#endif // ATLPT_IN_HPP
