#ifndef ATLPT_OUT_HPP
#define ATLPT_OUT_HPP

#include "AtLPT.hpp"

class AtLPT_Out : public AtLPT
{
public:
	AtLPT_Out();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	QScopedArrayPointer< quint16 > buffer;
	bool isOpen;
};

#endif // ATLPT_OUT_HPP
