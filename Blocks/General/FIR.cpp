#include "FIR.hpp"
#include "Array.hpp"

FIR::FIR() :
	Block( "FIR", "Filtr o skończonej odpowiedzi impulsowej", 1, 1, PROCESSING ),
	inputBuffer( NULL )
{}

bool FIR::start()
{
	if ( inputsCount() != outputsCount() )
		return false;
	settings->setRunMode( true );
	inputBuffer = new RingBuffer< float >[ inputsCount() ];
	inputSamples.resize( inputsCount() );
	setInputBuffer();
	return true;
}
void FIR::setSample( int input, float sample )
{
	inputSamples[ input ] = sample;
}
void FIR::exec( Array< Sample > &samples )
{
	mutex.lock();
	const float *fir_coeff_data = fir_coeff.data();
	for ( int i = 0 ; i < outputsCount() ; ++i )
	{
		float sum = 0.0;
		inputBuffer[ i ].set( inputSamples[ i ] );

		float *chunk1, *chunk2;
		int s1, s2;

		inputBuffer[ i ].getChunks( chunk1, s1, chunk2, s2 );

		int j, k;
		for ( j = 0 ; j < s1 ; ++j )
			sum += fir_coeff_data[ j ] * chunk1[ j ];
		k = j;
		for ( j = 0 ; j < s2 ; ++j, ++k )
			sum += fir_coeff_data[ k ] * chunk2[ j ];

		inputBuffer[ i ].advance();
		samples += ( Sample ){ getTarget( i ), sum };
	}
	mutex.unlock();
}
void FIR::stop()
{
	settings->setRunMode( false );
	delete[] inputBuffer;
	inputBuffer = NULL;
	inputSamples.clear();
}

Block *FIR::createInstance()
{
	FIR *block = new FIR;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true, new FIR_UI( *block ) );
	block->fir_coeff.fill( 1.0f, 1 );
	return block;
}

void FIR::serialize( QDataStream &ds ) const
{
	ds << fir_coeff;
}
void FIR::deSerialize( QDataStream &ds )
{
	ds >> fir_coeff;
}

void FIR::setInputBuffer()
{
	if ( inputBuffer )
		for ( int i = 0 ; i < inputsCount() ; ++i )
			inputBuffer[ i ].resize( fir_coeff.count() );
}

#include <QPlainTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QLayout>
#include <QLabel>

FIR_UI::FIR_UI( FIR &block ) :
	AdditionalSettings( block ),
	canUpdateFilter( true ),
	block( block )
{
	coeffE = new QPlainTextEdit;
	coeffE->setFont( QFont( "Monospace" ) );
	coeffE->setTabChangesFocus( true );

	filterLenL = new QLabel;

	QCheckBox *liveUpdateB = new QCheckBox( "Live update" );
	liveUpdateB->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );

	applyB = new QPushButton( "&Zastosuj" );
	applyB->setShortcut( QKeySequence( "Ctrl+S" ) );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( coeffE, 0, 0, 1, 2 );
	layout->addWidget( filterLenL, 1, 0, 1, 2 );
	layout->addWidget( liveUpdateB, 2, 0, 1, 1 );
	layout->addWidget( applyB, 2, 1, 1, 1 );
	layout->setMargin( 3 );

	connect( applyB, SIGNAL( clicked() ), this, SLOT( setFilter() ) );
	connect( liveUpdateB, SIGNAL( toggled( bool ) ), this, SLOT( toggleLiveUpdate( bool ) ) );
	connect( coeffE, SIGNAL( blockCountChanged( int ) ), this, SLOT( setFilterLenInfo( int ) ) );

	liveUpdateB->setChecked( true );

	setFilterLenInfo( coeffE->blockCount() );
}

void FIR_UI::prepare()
{
	QString fir_coeff;
	foreach ( float g, block.fir_coeff )
		fir_coeff += QString( "%1\n" ).arg( g );
	fir_coeff.chop( 1 );
	canUpdateFilter = false;
	coeffE->setPlainText( fir_coeff );
	canUpdateFilter = true;
}
bool FIR_UI::canClose()
{
	if ( coeffE->document()->isModified() )
		switch ( QMessageBox::question( this, block.getName(), "Czy chcesz zastosować zmiany?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel ) )
		{
			case QMessageBox::Yes:
				setFilter();
				break;
			case QMessageBox::No:
				break;
			default:
				return false;
		}
	coeffE->clear();
	AdditionalSettings::canClose();
	return true;
}

void FIR_UI::toggleLiveUpdate( bool l )
{
	applyB->setEnabled( !l );
	if ( !l )
		disconnect( coeffE, SIGNAL( textChanged() ), this, SLOT( setFilter() ) );
	else
	{
		setFilter();
		connect( coeffE, SIGNAL( textChanged() ), this, SLOT( setFilter() ) );
	}
}
void FIR_UI::setFilterLenInfo( int l )
{
	filterLenL->setText( "Długość filtru: " + QString::number( l - 1 ) );
}
void FIR_UI::setFilter()
{
	if ( canUpdateFilter && coeffE->document()->isModified() )
	{
		QStringList firLines = coeffE->toPlainText().split( '\n' );

		QVector< float > fir_coeff;
		fir_coeff.reserve( firLines.count() );
		for ( int i = firLines.count() - 1 ; i >= 0 ; --i )
		{
			const float val = firLines[ i ].toFloat();
			if ( !fir_coeff.isEmpty() || val != 0.0 )
				fir_coeff.prepend( val );
		}
		if ( fir_coeff.isEmpty() )
			fir_coeff += 0.0;

		block.mutex.lock();
		if ( block.fir_coeff != fir_coeff )
		{
			block.fir_coeff.clear();
			block.fir_coeff.resize( fir_coeff.count() );
			memcpy( block.fir_coeff.data(), fir_coeff.data(), fir_coeff.count() * sizeof( float ) );
			block.setInputBuffer();
		}
		block.mutex.unlock();

		coeffE->document()->setModified( false );
	}
}
