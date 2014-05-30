#include "VarDelay.hpp"
#include "Array.hpp"

VarDelay::VarDelay() :
	Block( "VarDelay", "Zmienne opóźnienie o n próbek na próbkę", 1, 1, PROCESSING ),
	delayedSamples( NULL ),
	minDelay( 10 ), maxDelay( 100 ), step( 0.001f )
{}

bool VarDelay::start()
{
	if ( inputsCount() != outputsCount() )
		return false;
	settings->setRunMode( true );
	delayedSamples = new RingBuffer< float >[ inputsCount() ];
	setDelayedSamples();
	return true;
}
void VarDelay::setSample( int input, float sample )
{
	mutex.lock();
	const int currDelay = ( int )delay;
	const int delayDiff = currDelay - lastDelay;
	if ( delayDiff > 0 )
		delayedSamples[ input ][ lastDelay ] = ( delayedSamples[ input ][ lastDelay - 1 ] + sample ) / 2.0f;
	else if ( delayDiff < 0 )
		delayedSamples[ input ][ currDelay ] = ( delayedSamples[ input ][ currDelay ] + sample ) / 2.0f;
	if ( delayDiff >= 0 )
		delayedSamples[ input ][ currDelay ] = sample;
	mutex.unlock();
}
void VarDelay::exec( Array< Sample > &samples )
{
	mutex.lock();
	lastDelay = ( int )delay;
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), delayedSamples[ i ].get() };
	if ( add && ( delay += step ) >= maxDelay )
		add = false;
	else if ( !add && ( delay -= step ) <= minDelay )
		add = true;
	mutex.unlock();
}
void VarDelay::stop()
{
	settings->setRunMode( false );
	delete[] delayedSamples;
	delayedSamples = NULL;
}

Block *VarDelay::createInstance()
{
	VarDelay *block = new VarDelay;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true, new VarDelayUI( *block ) );
	return block;
}

void VarDelay::serialize( QDataStream &ds ) const
{
	ds << minDelay << maxDelay << step;
}
void VarDelay::deSerialize( QDataStream &ds )
{
	ds >> minDelay >> maxDelay >> step;
}

void VarDelay::setDelayedSamples()
{
	if ( delayedSamples )
		for ( int i = 0 ; i < inputsCount() ; ++i )
			delayedSamples[ i ].resize( maxDelay + 1 );
	lastDelay = delay = minDelay;
	add = true;
}

#include <QPushButton>
#include <QSpinBox>
#include <QLayout>
#include <QLabel>

VarDelayUI::VarDelayUI( VarDelay &block ) :
	AdditionalSettings( block ),
	block( block )
{
	QLabel *minDelayL = new QLabel( "Minimalne opóźnienie: " );
	minDelayL->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );

	minDelayB = new QSpinBox;
	minDelayB->setSuffix( " próbek" );
	minDelayB->setRange( 0, 999999 );

	QLabel *maxDelayL = new QLabel( "Maksymalne opóźnienie: " );
	maxDelayL->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );

	maxDelayB = new QSpinBox;
	maxDelayB->setSuffix( " próbek" );
	maxDelayB->setRange( 0, 999999 );

	QLabel *stepL = new QLabel( "Zmiana opóźnienia: " );
	stepL->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );

	stepB = new QDoubleSpinBox;
	stepB->setSuffix( " próbek" );
	stepB->setDecimals( 5 );
	stepB->setRange( 0.00001, 1.0 );
	stepB->setSingleStep( 0.00001 );

	QPushButton *applyB = new QPushButton( "&Zastosuj" );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( minDelayL, 0, 0, 1, 1 );
	layout->addWidget( minDelayB, 0, 1, 1, 1 );
	layout->addWidget( maxDelayL, 1, 0, 1, 1 );
	layout->addWidget( maxDelayB, 1, 1, 1, 1 );
	layout->addWidget( stepL, 2, 0, 1, 1 );
	layout->addWidget( stepB, 2, 1, 1, 1 );
	layout->addWidget( applyB, 3, 0, 1, 2 );
	layout->setMargin( 3 );

	connect( applyB, SIGNAL( clicked() ), this, SLOT( setValue() ) );
}

void VarDelayUI::prepare()
{
	minDelayB->setValue( block.minDelay );
	maxDelayB->setValue( block.maxDelay );
	stepB->setValue( block.step );
}

void VarDelayUI::setValue()
{
	if ( block.minDelay != minDelayB->value() || block.maxDelay != maxDelayB->value() || block.step != ( float )stepB->value() )
	{
		block.minDelay = minDelayB->value();
		block.maxDelay = maxDelayB->value();
		block.step = stepB->value();
		block.mutex.lock();
		block.setDelayedSamples();
		block.mutex.unlock();
	}
}
