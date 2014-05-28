#include "Const.hpp"
#include "Array.hpp"

Const::Const() :
	Block( "Const", "Generuje stałą liczbę", 0, 1, SOURCE ),
	output( 1.0f )
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
	ds << output;
}
void Const::deSerialize( QDataStream &ds )
{
	ds >> output;
	setLabel();
}

void Const::setLabel()
{
	label = QString( "%1" ).arg( output );
	update();
}

#include <QDoubleSpinBox>
#include <QLayout>
#include <QLabel>

ConstUI::ConstUI( Const &block ) :
	AdditionalSettings( block ),
	block( block )
{
	QLabel *numberL = new QLabel( "Stała wartość: " );
	numberL->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );

	numberB = new QDoubleSpinBox;
	numberB->setDecimals( 4 );
	numberB->setSingleStep( 0.01 );
	numberB->setRange( -1000000.0, 1000000.0 );

	QHBoxLayout *layout = new QHBoxLayout( this );
	layout->addWidget( numberL );
	layout->addWidget( numberB );
	layout->setMargin( 3 );

	connect( numberB, SIGNAL( valueChanged( double ) ), this, SLOT( setValue( double ) ) );
}

void ConstUI::prepare()
{
	numberB->setValue( block.output );
}

void ConstUI::setValue( double v )
{
	block.output = v;
	block.setLabel();
}
