#include "RMS.hpp"
#include "Global.hpp"
#include "Array.hpp"

#include <math.h>

RMS::RMS() :
	Block( "RMS", "Oblicza wartość skuteczną, średnią, minimalną lub maksymalną sygnału", 1, 1, PROCESSING ),
	ms( 500 ),
	mode( MD_RMS )
{}

bool RMS::start()
{
	if ( inputsCount() != outputsCount() )
		return false;
	settings->setRunMode( true );
	partial_result.reset( new double[ inputsCount() ]() );
	result.reset( new float[ inputsCount() ] );
	for ( int i = 0 ; i < inputsCount() ; ++i )
		result[ i ] = 0.0f / 0.0f;
	calcNumSamples();
	pos = 0;
	return true;
}
void RMS::setSample( int input, float sample )
{
	int _pos = pos;
	switch ( mode )
	{
		case MD_RMS:
			partial_result[ input ] += sample * sample;
			break;
		case MD_AVG:
			partial_result[ input ] += sample;
			break;
		case MD_MAX:
			if ( sample > partial_result[ input ] || !_pos )
				partial_result[ input ] = sample;
			break;
		case MD_MIN:
			if ( sample < partial_result[ input ] || !_pos )
				partial_result[ input ] = sample;
			break;
	}
}
void RMS::exec( Array< Sample > &samples )
{
	quint8 _mode = mode;
	int _pos = pos.fetchAndAddRelaxed( 1 ) + 1;
	if ( _pos >= numSamples )
	{
		for ( int i = 0 ; i < inputsCount() ; ++i )
		{
			switch ( _mode )
			{
				case MD_RMS:
					result[ i ] = sqrt( partial_result[ i ] / _pos );
					break;
				case MD_AVG:
					result[ i ] = partial_result[ i ] / _pos;
					break;
				case MD_MAX:
				case MD_MIN:
					result[ i ] = partial_result[ i ];
					break;
			}
			partial_result[ i ] = 0.0;
		}
		pos = 0;
	}
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), result[ i ] };
}
void RMS::stop()
{
	settings->setRunMode( false );
	partial_result.reset();
	result.reset();
}

Block *RMS::createInstance()
{
	RMS *block = new RMS;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true, new RMS_UI( *block ) );
	return block;
}

void RMS::serialize( QDataStream &ds ) const
{
	ds << ms << mode;
}
void RMS::deSerialize( QDataStream &ds )
{
	ds >> ms >> mode;
	setLabel();
}

void RMS::setLabel()
{
	switch ( mode )
	{
		case MD_RMS:
			label = getName();
			break;
		case MD_AVG:
			label = "Avg";
			break;
		case MD_MAX:
			label = "Max";
			break;
		case MD_MIN:
			label = "Min";
			break;
	}
	update();
}
void RMS::calcNumSamples()
{
	numSamples = ms * Global::getSampleRate() / 1000;
	if ( !numSamples )
		numSamples = 1;
}

#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLayout>
#include <QLabel>

RMS_UI::RMS_UI( RMS &block ) :
	AdditionalSettings( block ),
	block( block )
{
	modeB = new QComboBox;
	modeB->addItems( QStringList() << "Wartość skuteczna" << "Wartość średnia" << "Wartość maksymalna" << "Wartość minimalna" );

	QLabel *msL = new QLabel( "Długość badanego sygnału: " );
	msL->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );

	msB = new QSpinBox;
	msB->setSuffix( " ms" );
	msB->setRange( 1, 10000 );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( modeB, 0, 0, 1, 2 );
	layout->addWidget( msL, 1, 0, 1, 1 );
	layout->addWidget( msB, 1, 1, 1, 1 );
	layout->setMargin( 3 );
}

void RMS_UI::prepare()
{
	modeB->setCurrentIndex( block.mode );
	msB->setValue( block.ms );
	connect( msB, SIGNAL( valueChanged( int ) ), this, SLOT( setValue( int ) ) );
	connect( modeB, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setMode( int ) ) );
}

void RMS_UI::setValue( int v )
{
	block.ms = v;
	block.calcNumSamples();
}
void RMS_UI::setMode( int m )
{
	block.mode = m;
	for ( int i = 0 ; i < block.inputsCount() ; ++i )
		block.partial_result[ i ] = 0.0f;
	block.pos = 0;
	block.setLabel();
}
