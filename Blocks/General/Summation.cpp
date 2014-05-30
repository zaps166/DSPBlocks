#include "Summation.hpp"
#include "Array.hpp"

Summation::Summation() :
	Block( "Summation", "Sumuje wejścia", 2, 1, PROCESSING ),
	oddChSign( false )
{}

bool Summation::start()
{
	settings->setRunMode( true );
	buffer.resize( inputsCount() );
	return true;
}
void Summation::setSample( int input, float sample )
{
	buffer[ input ] = sample;
}
void Summation::exec( Array< Sample > &samples )
{
	float output = buffer[ 0 ];
	for ( int i = 1 ; i < inputsCount() ; ++i )
		output += ( oddChSign && ( i & 1 ) ) ? -buffer[ i ] : buffer[ i ];
	samples += ( Sample ){ getTarget( 0 ), output };
}
void Summation::stop()
{
	settings->setRunMode( false );
	buffer.clear();
}

Block *Summation::createInstance()
{
	Summation *block = new Summation;
	block->settings = new Settings( *block, true, 2, maxIO, false, 0, 0, false, new SummationUI( *block ) );
	return block;
}

void Summation::serialize( QDataStream &ds ) const
{
	ds << oddChSign;
}
void Summation::deSerialize( QDataStream &ds )
{
	ds >> oddChSign;
}

#include <QCheckBox>
#include <QLayout>

SummationUI::SummationUI( Summation &block ) :
	AdditionalSettings( block ),
	block( block )
{
	oddChSignB = new QCheckBox( "Wejścia nieparzyste są odwracające" );

	QVBoxLayout *layout = new QVBoxLayout( this );
	layout->addWidget( oddChSignB );
	layout->setMargin( 3 );
}

void SummationUI::prepare()
{
	oddChSignB->setChecked( block.oddChSign );
	connect( oddChSignB, SIGNAL( clicked( bool ) ), this, SLOT( setValue( bool ) ) );
}

void SummationUI::setValue( bool b )
{
	block.oddChSign = b;
}
