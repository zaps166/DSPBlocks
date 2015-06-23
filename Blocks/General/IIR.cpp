#include "IIR.hpp"
#include "Array.hpp"

IIR::IIR() :
	Block( "IIR", "Filtr o nieskończonej odpowiedzi impulsowej", 1, 1, PROCESSING )
{}

bool IIR::start()
{
	if ( inputsCount() != outputsCount() )
		return false;
	settings->setRunMode( true );
	inputBuffer.reset( new RingBuffer< float >[ inputsCount() ] );
	outputBuffer.reset( new RingBuffer< double >[ inputsCount() ] );
	inputSamples.reset( new float[ inputsCount() ]() );
	setInputBuffer();
	return true;
}
void IIR::setSample( int input, float sample )
{
	inputSamples[ input ] = sample;
}
void IIR::exec( Array< Sample > &samples )
{
	mutex.lock();
	const double *ACoeff_data = Acoeff.constData();
	const double *BCoeff_data = Bcoeff.constData();
	for ( int i = 0 ; i < outputsCount() ; ++i )
	{
		double sum = 0.0;
		inputBuffer[ i ].set( inputSamples[ i ] );
		for ( int j = 0 ; j < Acoeff.count() ; ++j )
			sum += ACoeff_data[ j ] * inputBuffer[ i ][ j ] - BCoeff_data[ j ] * outputBuffer[ i ][ j ];
		inputBuffer[ i ].advance();
		outputBuffer[ i ].set( sum );
		outputBuffer[ i ].advance();
		samples += ( Sample ){ getTarget( i ), ( float )sum };
	}
	mutex.unlock();
}
void IIR::stop()
{
	settings->setRunMode( false );
	inputBuffer.reset();
	outputBuffer.reset();
	inputSamples.reset();
}

Block *IIR::createInstance()
{
	IIR *block = new IIR;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true, new IIR_UI( *block ) );
	block->Acoeff.fill( 1.0, 1 );
	block->Bcoeff.fill( 0.0, 1 );
	return block;
}

void IIR::serialize( QDataStream &ds ) const
{
	ds << Acoeff << Bcoeff;
}
void IIR::deSerialize( QDataStream &ds )
{
	ds >> Acoeff >> Bcoeff;
}

void IIR::setInputBuffer()
{
	if ( inputBuffer && outputBuffer && Acoeff.count() == Bcoeff.count() )
		for ( int i = 0 ; i < inputsCount() ; ++i )
		{
			inputBuffer[ i ].resize( Acoeff.count(), true );
			outputBuffer[ i ].resize( Bcoeff.count(), true );
		}
}

#include <QPlainTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QLayout>
#include <QLabel>

IIR_UI::IIR_UI( IIR &block ) :
	AdditionalSettings( block ),
	canUpdateFilter( true ),
	block( block )
{
	QLabel *AcoeffL = new QLabel( "Współczynniki A" );

	AcoeffE = new QPlainTextEdit;
	AcoeffE->setFont( QFont( "Monospace" ) );
	AcoeffE->setTabChangesFocus( true );

	QLabel *BcoeffL = new QLabel( "Współczynniki B" );

	BcoeffE = new QPlainTextEdit;
	BcoeffE->setFont( QFont( "Monospace" ) );
	BcoeffE->setTabChangesFocus( true );

	filterLenL = new QLabel;

	QCheckBox *liveUpdateB = new QCheckBox( "Live update" );
	liveUpdateB->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );

	applyB = new QPushButton( "&Zastosuj" );
	applyB->setShortcut( QKeySequence( "Ctrl+S" ) );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( AcoeffL, 0, 0, 1, 1 );
	layout->addWidget( BcoeffL, 0, 1, 1, 1 );
	layout->addWidget( AcoeffE, 1, 0, 1, 1 );
	layout->addWidget( BcoeffE, 1, 1, 1, 1 );
	layout->addWidget( filterLenL, 2, 0, 1, 1 );
	layout->addWidget( liveUpdateB, 2, 1, 1, 1 );
	layout->addWidget( applyB, 3, 0, 1, 2 );
	layout->setMargin( 3 );

	connect( applyB, SIGNAL( clicked() ), this, SLOT( setFilter() ) );
	connect( liveUpdateB, SIGNAL( toggled( bool ) ), this, SLOT( toggleLiveUpdate( bool ) ) );
	connect( AcoeffE, SIGNAL( blockCountChanged( int ) ), this, SLOT( setFilterLenInfo() ) );
	connect( BcoeffE, SIGNAL( blockCountChanged( int ) ), this, SLOT( setFilterLenInfo() ) );

	liveUpdateB->setChecked( true );

	setFilterLenInfo();
}

void IIR_UI::prepare()
{
	QString iir_coeff;

	canUpdateFilter = false;

	foreach ( double g, block.Acoeff )
		iir_coeff += QString( "%1\n" ).arg( g );
	iir_coeff.chop( 1 );
	AcoeffE->setPlainText( iir_coeff );

	iir_coeff.clear();

	foreach ( double g, block.Bcoeff )
		iir_coeff += QString( "%1\n" ).arg( g );
	iir_coeff.chop( 1 );

	BcoeffE->setPlainText( iir_coeff );

	canUpdateFilter = true;
}
bool IIR_UI::canClose()
{
	if ( AcoeffE->document()->isModified() || BcoeffE->document()->isModified() )
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
	AcoeffE->clear();
	BcoeffE->clear();
	AdditionalSettings::canClose();
	return true;
}

void IIR_UI::toggleLiveUpdate( bool l )
{
	applyB->setEnabled( !l );
	if ( !l )
	{
		disconnect( AcoeffE, SIGNAL( textChanged() ), this, SLOT( setFilter() ) );
		disconnect( BcoeffE, SIGNAL( textChanged() ), this, SLOT( setFilter() ) );
	}
	else
	{
		setFilter();
		connect( AcoeffE, SIGNAL( textChanged() ), this, SLOT( setFilter() ) );
		connect( BcoeffE, SIGNAL( textChanged() ), this, SLOT( setFilter() ) );
	}
}
void IIR_UI::setFilterLenInfo()
{
	filterLenL->setText( "Długość filtru: " + QString::number( qMax( AcoeffE->blockCount(), BcoeffE->blockCount() ) - 1 ) );
}
void IIR_UI::setFilter()
{
	if ( canUpdateFilter && ( AcoeffE->document()->isModified() || BcoeffE->document()->isModified() ) )
	{
		QStringList iir_lines[ 2 ] = { AcoeffE->toPlainText().split( '\n' ), BcoeffE->toPlainText().split( '\n' ) };

		QVector< double > coeff[ 2 ];
		for ( int j = 0 ; j < 2 ; ++j )
		{
			coeff[ j ].reserve( iir_lines[ j ].count() );
			for ( int i = iir_lines[ j ].count() - 1 ; i >= 0 ; --i )
			{
				const double val = iir_lines[ j ][ i ].toDouble();
				if ( !coeff[ j ].isEmpty() || val != 0.0 )
					coeff[ j ].prepend( val );
			}
			if ( coeff[ j ].isEmpty() )
				coeff[ j ].append( 0.0 );
		}

		/* Liczba współczynników A musi być taka sama jak współczynników B */
		if ( coeff[ 0 ].count() > coeff[ 1 ].count() )
			coeff[ 1 ].resize( coeff[ 0 ].count() );
		else if ( coeff[ 1 ].count() > coeff[ 0 ].count() )
		{
			int aS = coeff[ 0 ].count();
			coeff[ 0 ].resize( coeff[ 1 ].count() );
			for ( int i = aS ; i < coeff[ 0 ].count() ; ++i )
				coeff[ 0 ][ i ] = 1.0;
		}

		block.mutex.lock();
		if ( block.Acoeff != coeff[ 0 ] || block.Bcoeff != coeff[ 1 ] )
		{
			block.Acoeff.clear();
			block.Bcoeff.clear();
			block.Acoeff.resize( coeff[ 0 ].count() );
			block.Bcoeff.resize( coeff[ 1 ].count() );
			memcpy( block.Acoeff.data(), coeff[ 0 ].data(), coeff[ 0 ].count() * sizeof( double ) );
			memcpy( block.Bcoeff.data(), coeff[ 1 ].data(), coeff[ 1 ].count() * sizeof( double ) );
			block.setInputBuffer();
		}
		block.mutex.unlock();

		AcoeffE->document()->setModified( false );
		BcoeffE->document()->setModified( false );
	}
}
