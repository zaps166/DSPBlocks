#include "Quantizer.hpp"
#include "Array.hpp"

static float quantize( float val, quint8 bits )
{
	quint32 multiplier = 1 << ( bits - 1 );
	multiplier += multiplier - 1;
	if ( val > 1.0f )
		val = 1.0f;
	else if ( val < -1.0f )
		val = -1.0f;
	return ( round( ( val / 2.0f + 0.5f ) * multiplier ) / multiplier - 0.5f ) * 2.0f;
}

Quantizer::Quantizer() :
	Block( "Quantizer", "Kwantyzacja", 1, 1, PROCESSING ),
	bits( 8 )
{}

bool Quantizer::start()
{
	if ( inputsCount() != outputsCount() )
		return false;
	settings->setRunMode( true );
	buffer.resize( inputsCount() );
	return true;
}
void Quantizer::setSample( int input, float sample )
{
	buffer[ input ] = sample;
}
void Quantizer::exec( Array< Sample > &samples )
{
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), quantize( buffer[ i ], bits ) };
}
void Quantizer::stop()
{
	settings->setRunMode( false );
	buffer.clear();
}

Block *Quantizer::createInstance()
{
	Quantizer *block = new Quantizer;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true, new QuantizerUI( *block ) );
	block->setLabel();
	return block;
}

void Quantizer::serialize( QDataStream &ds ) const
{
	ds << bits;
}
void Quantizer::deSerialize( QDataStream &ds )
{
	ds >> bits;
	setLabel();
}

void Quantizer::setLabel()
{
	label = getName() + QString( "\n%1bit" ).arg( bits );
	update();
}

#include <QSpinBox>
#include <QLayout>

QuantizerUI::QuantizerUI( Quantizer &block ) :
	AdditionalSettings( block ),
	block( block )
{
	bitsB = new QSpinBox;
	bitsB->setPrefix( "Rozdzielczość: " );
	bitsB->setSuffix( "bit" );
	bitsB->setRange( 1, 32 );

	QVBoxLayout *layout = new QVBoxLayout( this );
	layout->addWidget( bitsB );
	layout->setMargin( 3 );
}

void QuantizerUI::prepare()
{
	bitsB->setValue( block.bits );
	connect( bitsB, SIGNAL( valueChanged( int ) ), this, SLOT( valueChanged( int ) ) );
}

void QuantizerUI::valueChanged( int val )
{
	block.bits = val;
	block.setLabel();
}
