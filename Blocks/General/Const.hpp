#ifndef CONST_HPP
#define CONST_HPP

#include "Block.hpp"

class Const : public Block
{
	friend class ConstUI;
public:
	Const();

	bool start();
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void setLabel();

	float output;
};

#include "Settings.hpp"

class QDoubleSpinBox;

class ConstUI : public AdditionalSettings
{
	Q_OBJECT
public:
	ConstUI( Const &block );

	void prepare();
private slots:
	void setValue( double v );
private:
	Const &block;

	QDoubleSpinBox *numberB;
};

#endif // CONST_HPP
