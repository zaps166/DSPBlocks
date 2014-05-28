#include "Counter.hpp"
#include "Array.hpp"

Counter::Counter() :
	Block( "Counter", "Zlicza impulsy i wystawia impuls przy przepełnieniu lub zliczoną wartość na wyjściu", 1, 1, PROCESSING ),
	slope( RISING_SLOPE ), mode( MD_COUNTER ),
	cnt_val( 100 )
{}

bool Counter::start()
{
	if ( inputsCount() != outputsCount() )
		return false;
	settings->setRunMode( true );
	lastState.resize( inputsCount() );
	currState.resize( inputsCount() );
	switch ( mode )
	{
		case MD_COUNTER:
			cnt.fill( 0, inputsCount() );
			break;
		case MD_TIMER:
			cnt.fill( cnt_val, inputsCount() );
			break;
	}
	return true;
}
void Counter::setSample( int input, float sample )
{
	if ( sample >= 0.48f )
		currState[ input ] = true;
	else if ( sample <= 0.16f )
		currState[ input ] = false;
}
void Counter::exec( Array< Sample > &samples )
{
	quint8 _slope = slope, _mode = mode;
	qint32 _cnt_val = cnt_val;
	for ( int i = 0 ; i < outputsCount() ; ++i )
	{
		bool pulse = false, act = false;
		if ( currState[ i ] != lastState[ i ] )
		{
			switch ( _slope )
			{
				case RISING_SLOPE:
					if ( !lastState[ i ] && currState[ i ] )
						act = true;
					break;
				case FALLING_SLOPE:
					if ( lastState[ i ] && !currState[ i ] )
						act = true;
					break;
				default:
					act = true;
			}
		}
		switch ( _mode )
		{
			case MD_COUNTER:
				if ( act && ++cnt[ i ] >= _cnt_val )
					cnt[ i ] = 0;
				samples += ( Sample ){ getTarget( i ), ( float )cnt[ i ] };
				break;
			case MD_TIMER:
				if ( act && !--cnt[ i ] )
				{
					cnt[ i ] = _cnt_val;
					pulse = true;
				}
				samples += ( Sample ){ getTarget( i ), ( float )pulse };
				break;
		}
	}
	currState.swap( lastState );
}
void Counter::stop()
{
	settings->setRunMode( false );
	lastState.clear();
	currState.clear();
	cnt.clear();
}

Block *Counter::createInstance()
{
	Counter *block = new Counter;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true, new CounterUI( *block ) );
	block->setLabel();
	return block;
}

void Counter::serialize( QDataStream &ds ) const
{
	ds << slope << cnt_val << mode;
}
void Counter::deSerialize( QDataStream &ds )
{
	ds >> slope >> cnt_val >> mode;
	setLabel();
}

void Counter::setLabel()
{
	switch ( mode )
	{
		case MD_COUNTER:
			label = getName();
			break;
		case MD_TIMER:
			label = "Timer";
			break;
	}
	update();
}

#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLayout>

CounterUI::CounterUI( Counter &block ) :
	AdditionalSettings( block ),
	block( block )
{
	slopeCB = new QComboBox;
	slopeCB->addItems( QStringList() << "Zbocze narastające" << "Zbocze opadające" << "Oba zbocza" );

	modeCB = new QComboBox;
	modeCB->addItems( QStringList() << "Wystaw na wyjściu ilość zliczonych impulsów" << "Wystaw na wyjściu impuls przy przepełnieniu" );

	cntValB = new QSpinBox;
	cntValB->setPrefix( "Zliczaj do: " );
	cntValB->setRange( 1, 0x7FFFFFFF /*INT_MAX*/ );

	QPushButton *applyB = new QPushButton( "&Zastosuj" );

	QVBoxLayout *layout = new QVBoxLayout( this );
	layout->addWidget( slopeCB );
	layout->addWidget( modeCB );
	layout->addWidget( cntValB );
	layout->addWidget( applyB );
	layout->setMargin( 3 );

	connect( applyB, SIGNAL( clicked() ), this, SLOT( apply() ) );
}

void CounterUI::prepare()
{
	slopeCB->setCurrentIndex( block.slope );
	modeCB->setCurrentIndex( block.mode );
	cntValB->setValue( block.cnt_val );
}

void CounterUI::apply()
{
	block.slope = slopeCB->currentIndex();
	block.mode = modeCB->currentIndex();
	block.cnt_val = cntValB->value();
	block.setLabel();
}
