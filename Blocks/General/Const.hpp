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
	bool integerSpinBox;
};

#include "Settings.hpp"

class QDoubleSpinBox;
class QCheckBox;

class ConstUI : public AdditionalSettings
{
	Q_OBJECT
public:
	ConstUI( Const &block );

	void prepare();
private slots:
	void setValue( double v );
	void integerToggled( bool b );
private:
	Const &block;

	QDoubleSpinBox *numberB;
	QCheckBox *integerB;
};

#endif // CONST_HPP
