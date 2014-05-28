#ifndef DELAY_HPP
#define DELAY_HPP

#include "Block.hpp"
#include "RingBuffer.hpp"

#include <QMutex>

class Delay : public Block
{
	friend class DelayUI;
public:
	Delay();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void setDelayedSamples();
	void setLabel();

	RingBuffer< float > *delayedSamples;
	QMutex mutex;
	int delay;
};

#include "Settings.hpp"

class QSpinBox;

class DelayUI : public AdditionalSettings
{
	Q_OBJECT
public:
	DelayUI( Delay &block );

	void prepare();
private slots:
	void setValue( int v );
private:
	Delay &block;

	QSpinBox *delayB;
};

#endif // DELAY_HPP
