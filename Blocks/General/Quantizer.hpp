#ifndef QUANTIZER_HPP
#define QUANTIZER_HPP

#include "Block.hpp"

class Quantizer : public Block
{
	friend class QuantizerUI;
public:
	Quantizer();

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

	QVector< float > buffer;
	quint8 bits;
};

#include "Settings.hpp"

class QSpinBox;

class QuantizerUI : public AdditionalSettings
{
	Q_OBJECT
public:
	QuantizerUI( Quantizer &block );

	void prepare();
private slots:
	void valueChanged( int val );
private:
	QSpinBox *bitsB;

	Quantizer &block;
};

#endif // QUANTIZER_HPP
