#ifndef GAIN_HPP
#define GAIN_HPP

#include "Block.hpp"

class Gain : public Block
{
	friend class GainUI;
public:
	Gain();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void setLabel();

	QScopedArrayPointer< float > buffer;
	float gain;
};

#include "Settings.hpp"

class QDoubleSpinBox;

class GainUI : public AdditionalSettings
{
	Q_OBJECT
public:
	GainUI( Gain &block );

	void prepare();
private slots:
	void setValue( double v );
private:
	Gain &block;

	QDoubleSpinBox *gainB;
};

#endif // GAIN_HPP
