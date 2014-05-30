#ifndef VARDELAY_HPP
#define VARDELAY_HPP

#include "Block.hpp"
#include "RingBuffer.hpp"

#include <QMutex>

class VarDelay : public Block
{
	friend class VarDelayUI;
public:
	VarDelay();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void setDelayedSamples();

	RingBuffer< float > *delayedSamples;
	int minDelay, maxDelay;
	float step, delay;
	int lastDelay;
	bool add;
	QMutex mutex;
};

#include "Settings.hpp"

class QDoubleSpinBox;
class QSpinBox;

class VarDelayUI : public AdditionalSettings
{
	Q_OBJECT
public:
	VarDelayUI( VarDelay &block );

	void prepare();
private slots:
	void setValue();
private:
	VarDelay &block;

	QSpinBox *minDelayB, *maxDelayB;
	QDoubleSpinBox *stepB;
};

#endif // VARDELAY_HPP
