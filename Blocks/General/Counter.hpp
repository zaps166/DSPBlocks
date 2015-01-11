#ifndef COUNTER_HPP
#define COUNTER_HPP

#include "Block.hpp"

class Counter : public Block
{
	friend class CounterUI;
public:
	Counter();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	enum { RISING_SLOPE, FALLING_SLOPE, BOTH_SLOPES };
	enum { MD_COUNTER, MD_TIMER };

	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void setLabel();

	QScopedArrayPointer< bool > lastState, currState;
	QScopedArrayPointer< qint32 > cnt;

	quint8 slope, mode;
	qint32 cnt_val;
};

#include "Settings.hpp"

class QComboBox;
class QSpinBox;

class CounterUI : public AdditionalSettings
{
	Q_OBJECT
public:
	CounterUI( Counter &block );

	void prepare();
private slots:
	void apply();
private:
	QComboBox *slopeCB, *modeCB;
	QSpinBox *cntValB;

	Counter &block;
};

#endif // COUNTER_HPP
