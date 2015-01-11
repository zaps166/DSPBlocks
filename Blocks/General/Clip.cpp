#include "Clip.hpp"
#include "Array.hpp"

Clip::Clip() :
	Block( "Clip", "Ucina sygnał", 1, 1, PROCESSING ),
	min( -1.0f ), max( 1.0f )
{}

bool Clip::start()
{
	if ( inputsCount() != outputsCount() )
		return false;
	settings->setRunMode( true );
	buffer.reset( new float[ inputsCount() ]() );
	return true;
}
void Clip::setSample( int input, float sample )
{
	buffer[ input ] = sample;
}
void Clip::exec( Array< Sample > &samples )
{
	for ( int i = 0 ; i < outputsCount() ; ++i )
	{
		if ( buffer[ i ] > max )
			buffer[ i ] = max;
		else if ( buffer[ i ] < min )
			buffer[ i ] = min;
		samples += ( Sample ){ getTarget( i ), buffer[ i ] };
	}
}
void Clip::stop()
{
	settings->setRunMode( false );
	buffer.reset();
}

Block *Clip::createInstance()
{
	Clip *block = new Clip;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true, new ClipUI( *block ) );
	block->setLabel();
	return block;
}

void Clip::serialize( QDataStream &ds ) const
{
	ds << min << max;
}
void Clip::deSerialize( QDataStream &ds )
{
	ds >> min >> max;
	setLabel();
}

void Clip::setLabel()
{
	label = QString( getName() + "\n[%1 - %2]" ).arg( min ).arg( max );
	update();
}

#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLayout>
#include <QLabel>

ClipUI::ClipUI( Clip &block ) :
	AdditionalSettings( block ),
	block( block )
{
	QLabel *minL = new QLabel( "Minimum: " );
	minL->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );

	QLabel *maxL = new QLabel( "Maximum: " );
	maxL->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );

	minB = new QDoubleSpinBox;
	minB->setDecimals( 4 );
	minB->setSingleStep( 0.01 );
	minB->setRange( -1000.0, 1000.0 );

	maxB = new QDoubleSpinBox;
	maxB->setDecimals( 4 );
	maxB->setSingleStep( 0.01 );
	maxB->setRange( -1000.0, 1000.0 );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( minL, 0, 0 );
	layout->addWidget( minB, 0, 1 );
	layout->addWidget( maxL, 1, 0 );
	layout->addWidget( maxB, 1, 1 );
	layout->setMargin( 3 );
}

void ClipUI::prepare()
{
	minB->setValue( block.min );
	maxB->setValue( block.max );
	connect( minB, SIGNAL( valueChanged( double ) ), this, SLOT( setMinValue( double ) ) );
	connect( maxB, SIGNAL( valueChanged( double ) ), this, SLOT( setMaxValue( double ) ) );
}

void ClipUI::setMinValue( double v )
{
	block.min = v;
	block.setLabel();
}
void ClipUI::setMaxValue( double v )
{
	block.max = v;
	block.setLabel();
}
