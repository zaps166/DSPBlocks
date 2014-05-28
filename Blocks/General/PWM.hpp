#ifndef PWM_HPP
#define PWM_HPP

#include "Block.hpp"

class PWM : public Block
{
public:
	PWM();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	float freq, duty;

	bool state;
	quint32 num_samples;
};

#endif // PWM_HPP
