#include "LADSPA.hpp"
#include "Global.hpp"
#include "Array.hpp"

#include <QDoubleSpinBox>
#include <QCheckBox>

#include <math.h>

#define BUFFER_SIZE 2

static inline LADSPA_Data getDefaultValue( bool isLogarithmic, LADSPA_Data lower, double val1, LADSPA_Data upper, double val2 )
{
	return isLogarithmic ? exp( log( lower ) * val1 + log( upper ) * val2 ) : ( lower * val1 + upper * val2 );
}

LADSPA::LADSPA( const QString &name, int numInputs, int numOutputs, const LADSPA_Descriptor &ld ) :
	Block( name, QString(), numInputs, numOutputs, PROCESSING ),
	multiinstances( true ),
	ld( ld )
{}

bool LADSPA::start()
{
	settings->setRunMode( true );
	buffer.resize( qMax( inputsCount(), outputsCount() ) );
	for ( int i = 0 ; i < buffer.count() ; ++i )
		buffer[ i ].resize( BUFFER_SIZE );
	int inPort = 0, outPort = 0;
	for ( int j = 0 ; j < buffer.count() ; ++j )
	{
		LADSPA_Handle ladspa_instance = ld.instantiate( &ld, Global::getSampleRate() );
		if ( !ladspa_instance )
			return false;
		int ctlPort = 0;
		for ( unsigned long i = 0 ; i < ld.PortCount ; ++i )
			if ( LADSPA_IS_PORT_AUDIO( ld.PortDescriptors[ i ] ) )
			{
				if ( LADSPA_IS_PORT_INPUT( ld.PortDescriptors[ i ] ) )
					ld.connect_port( ladspa_instance, i, buffer[ inPort++ ].data() );
				if ( LADSPA_IS_PORT_OUTPUT( ld.PortDescriptors[ i ] ) )
					ld.connect_port( ladspa_instance, i, buffer[ outPort++ ].data() );
			}
			else if ( LADSPA_IS_PORT_CONTROL( ld.PortDescriptors[ i ] ) )
				ld.connect_port( ladspa_instance, i, &dynamic_cast< LADSPA_UI * >( settings->getAdditionalSettings() )->control[ ctlPort++ ] );
		if ( ld.activate )
			ld.activate( ladspa_instance );
		ladspa_instances += ladspa_instance;
		if ( !multiinstances )
			break;
	}
	pos = 0;
	return true;
}
void LADSPA::setSample( int input, float sample )
{
	buffer[ input ][ pos ] = sample;
}
void LADSPA::exec( Array< Sample > &samples )
{
	if ( ++pos == BUFFER_SIZE )
	{
		for ( int i = 0 ; i < ladspa_instances.count() ; ++i )
			ld.run( ladspa_instances[ i ], buffer[ i ].count() );
		pos = 0;
	}
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), buffer[ i ][ pos ] };
}
void LADSPA::stop()
{
	settings->setRunMode( false );
	foreach ( LADSPA_Handle ladspa_instance, ladspa_instances )
	{
		if ( ld.deactivate )
			ld.deactivate( ladspa_instance );
		ld.cleanup( ladspa_instance );
	}
	ladspa_instances.clear();
	buffer.clear();
}

Block *LADSPA::createInstance()
{
	LADSPA *block = new LADSPA( getName(), inputsCount(), outputsCount(), ld );
	if ( inputsCount() == 1 && outputsCount() == 1 )
		block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true, new LADSPA_UI( *block ) );
	else
	{
		block->settings = new Settings( *block, false, 0, 0, false, 0, 0, false, new LADSPA_UI( *block ) );
		block->multiinstances = false;
	}
	return block;
}

void LADSPA::serialize( QDataStream &ds ) const
{
	QVector< LADSPA_Data > &control = dynamic_cast< LADSPA_UI * >( settings->getAdditionalSettings() )->control;
	if ( !control.isEmpty() )
		ds << control;
}
void LADSPA::deSerialize( QDataStream &ds )
{
	QList< QWidget * > &controlWList = dynamic_cast< LADSPA_UI * >( settings->getAdditionalSettings() )->controlWList;
	if ( !controlWList.isEmpty() )
	{
		QVector< LADSPA_Data > tmp_control;
		ds >> tmp_control;
		if ( tmp_control.count() == controlWList.count() )
			for ( int i = 0 ; i < tmp_control.count() ; ++i )
			{
				QDoubleSpinBox *valueB = qobject_cast< QDoubleSpinBox * >( controlWList[ i ] );
				QCheckBox *checkBox = qobject_cast< QCheckBox * >( controlWList[ i ] );
				if ( valueB )
					valueB->setValue( tmp_control[ i ] );
				else if ( checkBox )
					checkBox->setChecked( tmp_control[ i ] );
			}
		controlWList.clear();
	}
}

#include <QLayout>
#include <QSlider>
#include <QLabel>

LADSPA_UI::LADSPA_UI( LADSPA &block ) :
	AdditionalSettings( block ),
	block( block )
{
	const LADSPA_Descriptor &ld = block.ld;
	QGridLayout *layout = new QGridLayout( this );
	int row = 0;

	QLabel *label = new QLabel( QString( "Autor: %1\n" ).arg( ld.Maker ) + ( qstrcmp( ld.Copyright, "None" ) ? QString( "%1\n" ).arg( ld.Copyright ) : QString() ) );
	label->setAlignment( Qt::AlignTop );
	layout->addWidget( label, row++, 0, 1, 5 );

	QList< unsigned long > ladspa_descriptors_idx;
	for ( unsigned long i = 0 ; i < ld.PortCount ; ++i )
		if ( LADSPA_IS_PORT_CONTROL( ld.PortDescriptors[ i ] ) )
			ladspa_descriptors_idx += i;
	if ( !ladspa_descriptors_idx.isEmpty() )
	{
		int j = 0;
		control.resize( ladspa_descriptors_idx.count() );
		foreach ( int i, ladspa_descriptors_idx )
		{
			const LADSPA_PortRangeHintDescriptor hintDescr = ld.PortRangeHints[ i ].HintDescriptor;
			const int multiplier_srate = LADSPA_IS_HINT_SAMPLE_RATE( hintDescr ) ? Global::getSampleRate() : 1.0f;
			const LADSPA_Data lower = LADSPA_IS_HINT_BOUNDED_BELOW( hintDescr ) ? ld.PortRangeHints[ i ].LowerBound * multiplier_srate : -100.0f;
			const LADSPA_Data upper = LADSPA_IS_HINT_BOUNDED_ABOVE( hintDescr ) ? ld.PortRangeHints[ i ].UpperBound * multiplier_srate :  100.0f;
			LADSPA_Data &ctl = control[ j ];
			if ( LADSPA_IS_HINT_TOGGLED( hintDescr ) )
			{
				QCheckBox *checkBox = new QCheckBox( ld.PortNames[ i ] );
				checkBox->setProperty( "ctlPort", j );
				controlWList += checkBox;
				if ( !LADSPA_IS_HINT_DEFAULT_1( hintDescr ) )
					ctl = 0.0f;
				else
				{
					checkBox->setChecked( true );
					ctl = 1.0f;
				}
				connect( checkBox, SIGNAL( stateChanged( int ) ), this, SLOT( setValue( int ) ) );
				layout->addWidget( checkBox, row++, 0, 1, 5 );
			}
			else
			{
				const int multiplier = LADSPA_IS_HINT_INTEGER( hintDescr ) ? 1 : 1000;
				const bool isLogarithmic = LADSPA_IS_HINT_LOGARITHMIC( hintDescr );

				QSlider *slider = new QSlider( Qt::Horizontal );
				QDoubleSpinBox *valueB = new QDoubleSpinBox;
				if ( multiplier == 1 )
					valueB->setDecimals( 0 );
				else
				{
					valueB->setDecimals( 3 );
					valueB->setSingleStep( 0.001 );
				}
				controlWList += valueB;

				if ( LADSPA_IS_HINT_DEFAULT_MINIMUM( hintDescr ) )
					ctl = lower;
				else if ( LADSPA_IS_HINT_DEFAULT_LOW( hintDescr ) )
					ctl = getDefaultValue( isLogarithmic, lower, 0.75, upper, 0.25 );
				else if ( LADSPA_IS_HINT_DEFAULT_MIDDLE( hintDescr ) )
					ctl = getDefaultValue( isLogarithmic, lower, 0.5, upper, 0.5 );
				else if ( LADSPA_IS_HINT_DEFAULT_HIGH( hintDescr ) )
					ctl = getDefaultValue( isLogarithmic, lower, 0.25, upper, 0.75 );
				else if ( LADSPA_IS_HINT_DEFAULT_MAXIMUM( hintDescr ) )
					ctl = upper;
				else if ( LADSPA_IS_HINT_DEFAULT_0( hintDescr ) )
					ctl = 0.0f;
				else if ( LADSPA_IS_HINT_DEFAULT_1( hintDescr ) )
					ctl = 1.0f;
				else if ( LADSPA_IS_HINT_DEFAULT_100( hintDescr ) )
					ctl = 100.0f;
				else if ( LADSPA_IS_HINT_DEFAULT_440( hintDescr ) )
					ctl = 440.0f;

				slider->setRange( lower * multiplier, upper * multiplier );
				valueB->setRange( lower, upper );

				slider->setValue( ctl * multiplier );
				valueB->setValue( ctl );
				ctl = slider->value() / ( float )multiplier;

				connect( slider, SIGNAL( valueChanged( int ) ), this, SLOT( setValue( int ) ) );
				connect( valueB, SIGNAL( valueChanged( double ) ), this, SLOT( setValue( double ) ) );

				slider->setProperty( "second_control", qVariantFromValue( ( quintptr )valueB ) );
				valueB->setProperty( "second_control", qVariantFromValue( ( quintptr )slider ) );

				slider->setProperty( "multiplier", multiplier );
				valueB->setProperty( "multiplier", multiplier );

				slider->setProperty( "ctlPort", j );
				valueB->setProperty( "ctlPort", j );

				QLabel *labelN = new QLabel( ld.PortNames[ i ] + QString( ": " ) );
				labelN->setAlignment( Qt::AlignRight );
				layout->addWidget( labelN, row, 0, 1, 1 );
				layout->addWidget( valueB, row, 1, 1, 1 );
				layout->addWidget( new QLabel( QString::number( lower ) ), row, 2, 1, 1 );
				layout->addWidget( slider, row, 3, 1, 1 );
				layout->addWidget( new QLabel( QString::number( upper ) ), row, 4, 1, 1 );
				++row;
			}
			++j;
		}
	}
	layout->setMargin( 3 );
}

void LADSPA_UI::setValue( int v )
{
	LADSPA_Data &data = control[ sender()->property( "ctlPort" ).toInt() ];
	if ( qobject_cast< QCheckBox * >( sender() ) )
		data = v >> 1;
	else
	{
		const int multiplier = sender()->property( "multiplier" ).toInt();
		data = v / ( float )multiplier;
		( ( QDoubleSpinBox * )sender()->property( "second_control" ).value< quintptr >() )->setValue( v / ( double )multiplier );
	}
}
void LADSPA_UI::setValue( double v )
{
	control[ sender()->property( "ctlPort" ).toInt() ] = v;
	( ( QSlider * )sender()->property( "second_control" ).value< quintptr >() )->setValue( v * sender()->property( "multiplier" ).toInt() );
}
