#include "Sine.hpp"
#include "Array.hpp"

#include <math.h>

Sine::Sine() :
	Block( "Sine", "Generuje sinus lub prostokąt", 0, 1, SOURCE ),
	hz( 440.0f ), phase( 0.0f ),
	square( 0 )
{}

bool Sine::start()
{
	settings->setRunMode( true );
	lastPos = 0;
	return true;
}
void Sine::exec( Array< Sample > &samples )
{
	mutex.lock();
	float sine_sample = sin( 2.0f * M_PI * hz * lastPos / getSampleRate() - phase );
	switch ( square )
	{
		case 1:
			sine_sample = sine_sample >= 0.0f ? 1.0f : 0.0f;
			break;
		case 2:
			sine_sample = sine_sample >= 0.0f ? 1.0f : -1.0f;
			break;
	}
	++lastPos;
	setSinePos( hz, hz );
	mutex.unlock();
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), sine_sample };
}
void Sine::stop()
{
	settings->setRunMode( false );
}

Block *Sine::createInstance()
{
	Sine *block = new Sine;
	block->settings = new Settings( *block, false, 0, 0, true, 1, maxIO, false, new SineUI( *block ) );
	block->setLabel();
	return block;
}

void Sine::setSinePos( double hz1, double hz2 )
{
	if ( !hz1 || !hz2 )
		lastPos = 0.0;
	else
	{
		double k = hz1 * lastPos / getSampleRate();
		lastPos = ( k - ( int )k ) * getSampleRate() / hz2;
	}
}
void Sine::setLabel()
{
	switch ( square )
	{
		case 0:
			label = getName();
			break;
		case 1:
			label = "Half\nsquare";
			break;
		case 2:
			label = "Square";
			break;
	}
	label += QString( "\n%1Hz, %2°" ).arg( hz ).arg( phase * 180.0 / M_PI );
	update();
}

void Sine::serialize( QDataStream &ds ) const
{
	ds << hz << phase << square;
}
void Sine::deSerialize( QDataStream &ds )
{
	ds >> hz >> phase >> square;
	setLabel();
}

#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLayout>
#include <QSlider>
#include <QLabel>

SineUI::SineUI( Sine &block ) :
	AdditionalSettings( block ),
	block( block ),
	canChVal( true )
{
	hzS = new QSlider( Qt::Horizontal );

	QLabel *numberL = new QLabel( "Częstotliwość: " );
	numberL->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );

	hzB = new QDoubleSpinBox;
	hzB->setRange( 0.0, 500000.0 );
	hzB->setSuffix( " Hz" );

	QLabel *phaseL = new QLabel( "Przesunięcie fazowe: " );

	phaseB = new QDoubleSpinBox;
	phaseB->setRange( 0.0, 360.0 );
	phaseB->setDecimals( 1 );
	phaseB->setSuffix( "°" );

	squareB = new QCheckBox( "Prostokąt" );
	squareB->setTristate();

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( hzS, 0, 0, 1, 2 );
	layout->addWidget( numberL, 1, 0, 1, 1 );
	layout->addWidget( hzB, 1, 1, 1, 1 );
	layout->addWidget( phaseL, 2, 0, 1, 1 );
	layout->addWidget( phaseB, 2, 1, 1, 1 );
	layout->addWidget( squareB, 3, 0, 1, 2 );
	layout->setMargin( 3 );
}

void SineUI::prepare()
{
	hzS->setRange( 0.0, Block::getSampleRate() / 2 );
	hzS->setValue( block.hz );
	hzB->setValue( block.hz );
	phaseB->setValue( block.phase * 180.0 / M_PI );
	squareB->setCheckState( ( Qt::CheckState )block.square );
	connect( hzS, SIGNAL( valueChanged( int ) ), this, SLOT( setHz( int ) ) );
	connect( hzB, SIGNAL( valueChanged( double ) ), this, SLOT( setValue() ) );
	connect( phaseB, SIGNAL( valueChanged( double ) ), this, SLOT( setValue() ) );
	connect( squareB, SIGNAL( stateChanged( int ) ), this, SLOT( setValue() ) );
}

void SineUI::setHz( int hz )
{
	if ( canChVal )
		hzB->setValue( hz );
}
void SineUI::setValue()
{
	double hz = hzB->value();
	canChVal = false;
	hzS->setValue( hz );
	canChVal = true;
	block.mutex.lock();
	block.phase = phaseB->value() * M_PI / 180.0f;
	block.setSinePos( block.hz, hz );
	block.square = squareB->checkState();
	if ( block.hz != hz )
	{
		QList< Sine * > sineBlocks = block.getBlocksByType< Sine >();
		foreach ( Sine *sine, sineBlocks )
			if ( sine != &block && sine->hz == hz )
			{
				block.lastPos = sine->lastPos;
				break;
			}
		block.hz = hz;
	}
	block.mutex.unlock();
	block.setLabel();
}
