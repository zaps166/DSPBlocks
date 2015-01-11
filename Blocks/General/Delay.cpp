#include "Delay.hpp"
#include "Array.hpp"

Delay::Delay() :
	Block( "Delay", "Opóźnienie o n próbek", 1, 1, PROCESSING ),
	delay( 1 )
{}

bool Delay::start()
{
	if ( inputsCount() != outputsCount() )
		return false;
	settings->setRunMode( true );
	delayedSamples.reset( new RingBuffer< float >[ inputsCount() ] );
	setDelayedSamples();
	return true;
}
void Delay::setSample( int input, float sample )
{
	mutex.lock();
	delayedSamples[ input ][ delay ] = sample;
	mutex.unlock();
}
void Delay::exec( Array< Sample > &samples )
{
	mutex.lock();
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), delayedSamples[ i ].get() };
	mutex.unlock();
}
void Delay::stop()
{
	settings->setRunMode( false );
	delayedSamples.reset();
}

Block *Delay::createInstance()
{
	Delay *block = new Delay;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true, new DelayUI( *block ) );
	block->setLabel();
	return block;
}

void Delay::serialize( QDataStream &ds ) const
{
	ds << delay;
}
void Delay::deSerialize( QDataStream &ds )
{
	ds >> delay;
	setLabel();
}

void Delay::setDelayedSamples()
{
	if ( delayedSamples )
		for ( int i = 0 ; i < inputsCount() ; ++i )
			delayedSamples[ i ].resize( delay + 1 );
}
void Delay::setLabel()
{
	label = QString( getName() + "\nz-%1" ).arg( delay );
	update();
}

#include <QSpinBox>
#include <QLayout>
#include <QLabel>

DelayUI::DelayUI( Delay &block ) :
	AdditionalSettings( block ),
	block( block )
{
	QLabel *delayL = new QLabel( "Opóźnienie: " );
	delayL->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );

	delayB = new QSpinBox;
	delayB->setSuffix( " próbek" );
	delayB->setRange( 0, 999999 );

	QHBoxLayout *layout = new QHBoxLayout( this );
	layout->addWidget( delayL );
	layout->addWidget( delayB );
	layout->setMargin( 3 );

	connect( delayB, SIGNAL( valueChanged( int ) ), this, SLOT( setValue( int ) ) );
}

void DelayUI::prepare()
{
	delayB->setValue( block.delay );
}

void DelayUI::setValue( int v )
{
	if ( block.delay != v )
	{
		block.delay = v;
		block.mutex.lock();
		block.setDelayedSamples();
		block.mutex.unlock();
		block.setLabel();
	}
}
