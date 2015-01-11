#include "Logic.hpp"
#include "Array.hpp"

Logic::Logic( LogicType logicType, const QString &name, const QString &description ) :
	Block( name, description, ( logicType == VALUE || logicType == BUFFER || logicType == NOT ) ? 1 : 2, 1, logicType == VALUE ? SOURCE : PROCESSING ),
	logicType( logicType ),
	lo( 0.16f ), hi( 0.48f ),
	state( false )
{}

bool Logic::start()
{
	settings->setRunMode( true );
	if ( logicType != VALUE )
		buffer.reset( new bool[ inputsCount() ]() );
	return true;
}
void Logic::setSample( int input, float sample )
{
	if ( lo < hi )
	{
		if ( sample <= lo )
			buffer[ input ] = false;
		else if ( sample >= hi )
			buffer[ input ] = true;
	}
}
void Logic::exec( Array< Sample > &samples )
{
	if ( logicType == VALUE )
	{
		samples += ( Sample ){ getTarget( 0 ), ( float )state };
		return;
	}
	bool output = buffer[ 0 ];
	quint32 i;
	switch ( logicType )
	{
		case AND:
		case NAND:
			for ( i = 1 ; i < inputsCount() ; ++i )
				output &= buffer[ i ];
			break;
		case OR:
		case NOR:
			for ( i = 1 ; i < inputsCount() ; ++i )
				output |= buffer[ i ];
			break;
		case XOR:
		case XNOR:
			for ( i = 1 ; i < inputsCount() ; ++i )
				output ^= buffer[ i ];
			break;
		default:
			break;
	}
	switch ( logicType )
	{
		case NOT:
		case NAND:
		case NOR:
		case XNOR:
			output = !output;
		default:
			break;
	}
	samples += ( Sample ){ getTarget( 0 ), ( float )output };
}
void Logic::stop()
{
	settings->setRunMode( false );
	if ( logicType != VALUE )
		buffer.reset();
}

Block *Logic::createInstance()
{
	Logic *block = new Logic( logicType, getName(), getDescription() );
	block->settings = new Settings( *block, block->inputsCount() > 1, block->inputsCount(), block->inputsCount() == 1 ? 1 : maxIO, false, 1, 1, false, new Logic_UI( *block ) );
	return block;
}

void Logic::serialize( QDataStream &ds ) const
{
	ds << lo << hi;
	if ( logicType == VALUE )
		ds << state;
}
void Logic::deSerialize( QDataStream &ds )
{
	ds >> lo >> hi;
	if ( logicType == VALUE )
		ds >> state;
}

#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLayout>
#include <QLabel>

Logic_UI::Logic_UI( Logic &block ) :
	AdditionalSettings( block ),
	block( block )
{
	QGridLayout *layout = new QGridLayout( this );
	if ( block.logicType == Logic::VALUE )
	{
		stateB = new QCheckBox( "Stan wyjścia" );
		layout->addWidget( stateB );
	}
	else
	{
		QLabel *loL = new QLabel( "Maksymalna wartość stanu niskiego: " );

		loB = new QDoubleSpinBox;
		loB->setRange( 0.0, 1000.0 );
		loB->setSingleStep( 0.01 );

		QLabel *hiL = new QLabel( "Minimalna wartość stanu wysokiego: " );

		hiB = new QDoubleSpinBox;
		hiB->setRange( 0.01, 1000.0 );
		hiB->setSingleStep( 0.01 );

		layout->addWidget( loL, 0, 0 );
		layout->addWidget( loB, 0, 1 );
		layout->addWidget( hiL, 1, 0 );
		layout->addWidget( hiB, 1, 1 );
	}
}

void Logic_UI::prepare()
{
	if ( block.logicType == Logic::VALUE )
	{
		stateB->setChecked( block.state );
		connect( stateB, SIGNAL( clicked( bool ) ), this, SLOT( setState( bool ) ) );
	}
	else
	{
		loB->setValue( block.lo );
		hiB->setValue( block.hi );
		connect( loB, SIGNAL( valueChanged( double ) ), this, SLOT( setLo( double ) ) );
		connect( hiB, SIGNAL( valueChanged( double ) ), this, SLOT( setHi( double ) ) );
	}
}

void Logic_UI::setLo( double val )
{
	block.lo = val;
}
void Logic_UI::setHi( double val )
{
	block.hi = val;
}
void Logic_UI::setState( bool s )
{
	block.state = s;
}
