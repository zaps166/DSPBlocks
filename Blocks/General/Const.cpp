#include "Const.hpp"
#include "Array.hpp"

Const::Const() :
	Block( "Const", "Generuje stałą liczbę", 0, 1, SOURCE ),
	output( 1.0f ),
	integerSpinBox( false )
{}

bool Const::start()
{
	settings->setRunMode( true );
	return true;
}
void Const::exec( Array< Sample > &samples )
{
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), output };
}
void Const::stop()
{
	settings->setRunMode( false );
}

Block *Const::createInstance()
{
	Const *block = new Const;
	block->settings = new Settings( *block, false, 0, 0, true, 1, maxIO, false, new ConstUI( *block ) );
	block->setLabel();
	return block;
}

void Const::serialize( QDataStream &ds ) const
{
	ds << output << integerSpinBox;
}
void Const::deSerialize( QDataStream &ds )
{
	ds >> output >> integerSpinBox;
	setLabel();
}

void Const::setLabel()
{
	label = QString( "%1" ).arg( output );
	update();
}

#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLayout>
#include <QLabel>

ConstUI::ConstUI( Const &block ) :
	AdditionalSettings( block ),
	block( block )
{
	QLabel *numberL = new QLabel( "Stała wartość: " );
	numberL->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );

	numberB = new QDoubleSpinBox;
	numberB->setRange( -1000000.0, 1000000.0 );

	integerB = new QCheckBox( "Liczby całkowite" );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( numberL, 0, 0, 1, 1 );
	layout->addWidget( numberB, 0, 1, 1, 1 );
	layout->addWidget( integerB, 1, 0, 1, 2 );
	layout->setMargin( 3 );
}

void ConstUI::prepare()
{
	numberB->setValue( block.output );
	integerB->setChecked( block.integerSpinBox );

	connect( numberB, SIGNAL( valueChanged( double ) ), this, SLOT( setValue( double ) ) );
	connect( integerB, SIGNAL( toggled( bool ) ), this, SLOT( integerToggled( bool ) ) );

	integerToggled( integerB->isChecked() );
}

void ConstUI::setValue( double v )
{
	block.output = v;
	block.setLabel();
}
void ConstUI::integerToggled( bool b )
{
	numberB->setDecimals( b ? 0 : 4 );
	numberB->setSingleStep( b ? 1.0 : 0.01 );
	block.integerSpinBox = b;
}
