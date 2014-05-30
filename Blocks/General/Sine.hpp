#ifndef SINE_HPP
#define SINE_HPP

#include "Block.hpp"

#include <QMutex>

class Sine : public Block
{
	friend class SineUI;
public:
	Sine();

	bool start();
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void setSinePos( double hz1, double hz2 );
	void setLabel();

	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	double lastPos;
	float hz, phase;
	quint8 square;
	QMutex mutex;
};

#include "Settings.hpp"

class QDoubleSpinBox;
class QCheckBox;
class QSlider;

class SineUI : public AdditionalSettings
{
	Q_OBJECT
public:
	SineUI( Sine &block );

	void prepare();
private slots:
	void setHz( int hz );
	void setValue();
private:
	Sine &block;

	bool canChVal;

	QSlider *hzS;
	QDoubleSpinBox *hzB, *phaseB;
	QCheckBox *squareB;
};

#endif // SINE_HPP
