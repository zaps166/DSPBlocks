#include "Gain.hpp"
#include "Array.hpp"

Gain::Gain() :
	Block( "Gain", "Wzmocnienie sygnału", 1, 1, PROCESSING ),
	gain( 1.0f )
{}

bool Gain::start()
{
	if ( inputsCount() != outputsCount() )
		return false;
	settings->setRunMode( true );
	buffer.reset( new float[ inputsCount() ]() );
	return true;
}
void Gain::setSample( int input, float sample )
{
	buffer[ input ] = sample;
}
void Gain::exec( Array< Sample > &samples )
{
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), buffer[ i ] * gain };
}
void Gain::stop()
{
	settings->setRunMode( false );
	buffer.reset();
}

Block *Gain::createInstance()
{
	Gain *block = new Gain;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true, new GainUI( *block ) );
	block->setLabel();
	return block;
}

void Gain::serialize( QDataStream &ds ) const
{
	ds << gain;
}
void Gain::deSerialize( QDataStream &ds )
{
	ds >> gain;
	setLabel();
}

void Gain::setLabel()
{
	label = QString( getName() + "\n%1x" ).arg( gain );
	update();
}

#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLayout>
#include <QLabel>

GainUI::GainUI( Gain &block ) :
	AdditionalSettings( block ),
	block( block )
{
	QLabel *gainL = new QLabel( "Wzmocnienie: " );
	gainL->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );

	gainB = new QDoubleSpinBox;
	gainB->setDecimals( 4 );
	gainB->setSingleStep( 0.01 );
	gainB->setRange( -1000.0, 1000.0 );

	QHBoxLayout *layout = new QHBoxLayout( this );
	layout->addWidget( gainL );
	layout->addWidget( gainB );
	layout->setMargin( 3 );
}

void GainUI::prepare()
{
	gainB->setValue( block.gain );
	connect( gainB, SIGNAL( valueChanged( double ) ), this, SLOT( setValue( double ) ) );
}

void GainUI::setValue( double v )
{
	block.gain = v;
	block.setLabel();
}
