#ifndef SUMMATION_HPP
#define SUMMATION_HPP

#include "Block.hpp"

class Summation : public Block
{
	friend class SummationUI;
public:
	Summation();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	QScopedArrayPointer< float > buffer;
	bool oddChSign;
};

#include "Settings.hpp"

class QCheckBox;

class SummationUI : public AdditionalSettings
{
	Q_OBJECT
public:
	SummationUI( Summation &block );

	void prepare();
private slots:
	void setValue( bool b );
private:
	Summation &block;

	QCheckBox *oddChSignB;
};

#endif // SUMMATION_HPP
