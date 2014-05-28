#ifndef CLIP_HPP
#define CLIP_HPP

#include "Block.hpp"

class Clip : public Block
{
	friend class ClipUI;
public:
	Clip();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void setLabel();

	QVector< float > buffer;
	float min, max;
};

#include "Settings.hpp"

class QDoubleSpinBox;

class ClipUI : public AdditionalSettings
{
	Q_OBJECT
public:
	ClipUI( Clip &block );

	void prepare();
private slots:
	void setMinValue( double v );
	void setMaxValue( double v );
private:
	Clip &block;

	QDoubleSpinBox *minB, *maxB;
};

#endif // CLIP_HPP
