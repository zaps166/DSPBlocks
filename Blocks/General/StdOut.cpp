#include "StdOut.hpp"
#include "Settings.hpp"

#include <stdio.h>

StdOut::StdOut() :
	Block( "Stdout writer", "Wypisuje wartości próbek na standardowe wyjście", 1, 0, SINK ),
	boolean( false )
{}

bool StdOut::start()
{
	settings->setRunMode( true );
	buffer.resize( inputsCount() );
	return true;
}
void StdOut::setSample( int input, float sample )
{
	buffer[ input ] = sample;
}
void StdOut::exec( Array< Sample > & )
{
	if ( lastBuffer.count() != buffer.count() || memcmp( lastBuffer.data(), buffer.data(), buffer.count() * sizeof( float ) ) )
	{
		bool o = false;
		for ( int i = 0 ; i < inputsCount() ; ++i )
		{
			if ( buffer[ i ] != buffer[ i ] ) //NaN
				continue;
			if ( i )
				putchar( ' ' );
			if ( boolean && buffer[ i ] == 0.0f )
				printf( "       LO" );
			else if ( boolean && buffer[ i ] == 1.0f )
				printf( "       HI" );
			else
				printf( "%9.5f", buffer[ i ] );
			o = true;
		}
		if ( o )
		{
			putchar( '\n' );
			fflush( stdout );
			lastBuffer = buffer;
		}
	}
}
void StdOut::stop()
{
	buffer.clear();
	lastBuffer.clear();
	settings->setRunMode( false );
}

Block *StdOut::createInstance()
{
	StdOut *block = new StdOut;
	block->settings = new Settings( *block, true, 1, maxIO, false, 0, 0, false, new StdOutUI( *block ) );
	return block;
}

void StdOut::serialize( QDataStream &ds ) const
{
	ds << boolean;
}
void StdOut::deSerialize( QDataStream &ds )
{
	ds >> boolean;
}

#include <QCheckBox>
#include <QLayout>

StdOutUI::StdOutUI( StdOut &block ) :
	AdditionalSettings( block ),
	block( block )
{
	booleanB = new QCheckBox( "Odczytuj 0.0 i 1.0 jako stany wyjść" );

	QVBoxLayout *layout = new QVBoxLayout( this );
	layout->addWidget( booleanB );
}

void StdOutUI::prepare()
{
	booleanB->setChecked( block.boolean );
	connect( booleanB, SIGNAL( clicked( bool ) ), this, SLOT( setValue( bool ) ) );
}

void StdOutUI::setValue( bool b )
{
	block.boolean = b;
}
