#include "Math.hpp"
#include "Array.hpp"

#include "math.h"

static QStringList mathOps = QStringList() << "Nop" << "Sin" << "Cos" << "Tan" << "Sinh" << "Cosh" << "Tanh" << "ASin" << "ACos" << "ATan" << "Sqrt" << "Sqr" << "Exp" << "Log10" << "Log2" << "Ln" << "Abs";

static double nop( double v )
{
	return v;
}
static double sqr( double v )
{
	return v * v;
}

Math::Math() :
	Block( "Math", "Funkcje matematyczne", 1, 1, PROCESSING ),
	opcode( 0 )
{}

bool Math::start()
{
	if ( inputsCount() != outputsCount() )
		return false;
	output.fill( 0.0f, inputsCount() );
	settings->setRunMode( true );
	setOperation();
	return true;
}
void Math::setSample( int input, float sample )
{
	output[ input ] = sample;
}
void Math::exec( Array< Sample > &samples )
{
	MathFunc _math_func = math_func;
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), ( float )_math_func( output[ i ] ) };
}
void Math::stop()
{
	settings->setRunMode( false );
	output.clear();
}

Block *Math::createInstance()
{
	Math *block = new Math;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true, new MathUI( *block ) );
	block->setLabel();
	return block;
}

void Math::serialize( QDataStream &ds ) const
{
	ds << opcode;
}
void Math::deSerialize( QDataStream &ds )
{
	ds >> opcode;
	setLabel();
}

void Math::setOperation()
{
	switch ( opcode )
	{
		case 1:
			math_func = sin;
			break;
		case 2:
			math_func = cos;
			break;
		case 3:
			math_func = tan;
			break;
		case 4:
			math_func = sinh;
			break;
		case 5:
			math_func = cosh;
			break;
		case 6:
			math_func = tanh;
			break;
		case 7:
			math_func = asin;
			break;
		case 8:
			math_func = acos;
			break;
		case 9:
			math_func = atan;
			break;
		case 10:
			math_func = sqrt;
			break;
		case 11:
			math_func = sqr;
			break;
		case 12:
			math_func = exp;
			break;
		case 13:
			math_func = log10;
			break;
		case 14:
			math_func = log2;
			break;
		case 15:
			math_func = log;
			break;
		case 16:
			math_func = fabs;
			break;
		default:
			math_func = nop;
			opcode = 0;
	}
}
void Math::setLabel()
{
	label = mathOps[ opcode >= mathOps.count() ? 0 : opcode ];
	update();
}

#include <QComboBox>
#include <QLayout>
#include <QLabel>

MathUI::MathUI( Math &block ) :
	AdditionalSettings( block ),
	block( block )
{
	QLabel *opL = new QLabel( "Operacja: " );
	opL->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );

	opB = new QComboBox;
	opB->addItems( mathOps );

	QHBoxLayout *layout = new QHBoxLayout( this );
	layout->addWidget( opL );
	layout->addWidget( opB );
	layout->setMargin( 3 );
}

void MathUI::prepare()
{
	opB->setCurrentIndex( block.opcode );
	connect( opB, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setOp( int ) ) );
}

void MathUI::setOp( int op )
{
	if ( block.opcode != op )
	{
		block.opcode = op;
		block.setOperation();
		block.setLabel();
	}
}
