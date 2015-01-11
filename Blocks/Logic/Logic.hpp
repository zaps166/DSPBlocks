#ifndef LOGIC_HPP
#define LOGIC_HPP

#include "Block.hpp"

class Logic : public Block
{
	friend class Logic_UI;
public:
	enum LogicType { VALUE, BUFFER, NOT, AND, NAND, OR, NOR, XOR, XNOR };

	Logic( LogicType logicType, const QString &name, const QString &description );

	bool start();
	void setSample( int in_buffer, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	QScopedArrayPointer< bool > buffer;
	LogicType logicType;
	float lo, hi;
	bool state;
};

#include "Settings.hpp"

class QDoubleSpinBox;
class QCheckBox;

class Logic_UI : public AdditionalSettings
{
	Q_OBJECT
public:
	Logic_UI( Logic &block );

	void prepare();
private slots:
	void setLo( double val );
	void setHi( double val );
	void setState( bool s );
private:
	QDoubleSpinBox *loB, *hiB;
	QCheckBox *stateB;

	Logic &block;
};

#endif // LOGIC_HPP
