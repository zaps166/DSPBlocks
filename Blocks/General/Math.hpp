#ifndef MATH_HPP
#define MATH_HPP

#include "Block.hpp"

class Math : public Block
{
	friend class MathUI;
public:
	Math();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void setOperation();
	void setLabel();

	typedef double ( *MathFunc )( double );
	MathFunc math_func;

	QScopedArrayPointer< float > buffer;
	quint8 opcode;
};

#include "Settings.hpp"

class QComboBox;

class MathUI : public AdditionalSettings
{
	Q_OBJECT
public:
	MathUI( Math &block );

	void prepare();
private slots:
	void setOp( int op );
private:
	Math &block;

	QComboBox *opB;
};

#endif // MATH_HPP
