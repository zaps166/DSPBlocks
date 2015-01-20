#include "Scope.hpp"
#include "Global.hpp"

#include <QCloseEvent>
#include <QPainter>
#include <QDebug>

static inline qreal clip( double v )
{
	if ( v < -1.0 )
		return -1.0;
	if ( v > 1.0 )
		return 1.0;
	if ( v != v ) //NaN
		v = 0.0;
	return v;
}

Scope::Scope() :
	Block( "Scope", "Oscyloskop", 1, 1, SINK ),
	drawThr( *this ),
	interp( 1 ),
	samplesVisible( 1024 ),
	scale( 1.0f ),
	xy( false ),
	fallingSlope( false ), triggerChn( 0 ), triggerPos( 0.2f ),
	cantClose( false )
{
	setStyle( &style );
	setWindowTitle( "Oscyloskop" );
	setPalette( Qt::black );
	setMouseTracking( true );
}

bool Scope::start()
{
	settings->setRunMode( true );
	cantClose = true;
	QWidget::show();

	inBuffer.resize( inputsCount() );
	lastInBuffer.resize( inputsCount() );
	outBuffer.resize( inputsCount() );
	lastOutBuffer.resize( inputsCount() );
	setBuffers();

	drawThr.start();

	return true;
}
void Scope::setSample( int input, float sample )
{
	execMutex.lock();
	inBuffer[ input ][ buffPos ] = sample;
	execMutex.unlock();
}
void Scope::exec( Array< Sample > & )
{
	execMutex.lock();
	++buffPos;
	if ( ( xy && buffPos == inBuffer[ 0 ].count() ) || ( !xy && buffPos == samplesVisible ) )
	{
		if ( drawMutex.tryLock() )
		{
			if ( !xy )
			{
				qSwap( lastOutBuffer, swapLastInBuffer ? lastInBuffer : outBuffer );
				swapLastInBuffer = false;
			}
			qSwap( inBuffer, outBuffer );
			bufferReady = true;
			drawCond.wakeOne();
			drawMutex.unlock();
		}
		else if ( !xy )
		{
			qSwap( lastInBuffer, inBuffer );
			swapLastInBuffer = true;
		}
		buffPos = 0;
	}
	execMutex.unlock();
}
void Scope::stop()
{
	settings->setRunMode( false );
	drawThr.stop();
	cantClose = false;
	inBuffer.clear();
	lastInBuffer.clear();
	outBuffer.clear();
	lastOutBuffer.clear();
}

Block *Scope::createInstance()
{
	Scope *block = new Scope;
	block->settings = new Settings( *block, true, 1, maxIO, false, 0, 0, false, new ScopeUI( *block ) );
	return block;
}

void Scope::serialize( QDataStream &ds ) const
{
	ds << xy << interp << samplesVisible << scale << fallingSlope << triggerChn << triggerPos;
}
void Scope::deSerialize( QDataStream &ds )
{
	ds >> xy >> interp >> samplesVisible >> scale >> fallingSlope >> triggerChn >> triggerPos;
}

void Scope::inputsCountChanged( int num )
{
	qobject_cast< ScopeUI * >( settings->getAdditionalSettings() )->inputsCountChanged( num );
}

void Scope::setBuffers()
{
	int size = xy ? ( Global::getSampleRate() * Global::getPeriod() ) : samplesVisible;
	for ( int i = 0 ; i < inBuffer.count() ; ++i )
	{
		inBuffer[ i ].clear();
		lastInBuffer[ i ].clear();
		outBuffer[ i ].clear();
		lastOutBuffer[ i ].clear();

		inBuffer[ i ].resize( size );
		lastInBuffer[ i ].resize( size );
		outBuffer[ i ].resize( size );
		lastOutBuffer[ i ].resize( size );
	}
	bufferReady = swapLastInBuffer = false;
	buffPos = triggerHold = 0;
}

void Scope::draw()
{
	if ( xy )
	{
		paintMutex.lock();
		paths.clear();
		paths.resize( 1 );
		paths[ 0 ].moveTo( clip( outBuffer[ 0 ][ 0 ] * scale ) + 1.0, -clip( outBuffer[ 1 ][ 0 ] * scale ) + 1.0 );
		for ( int j = 1 ; j < outBuffer[ 0 ].count() ; ++j )
			paths[ 0 ].lineTo( clip( outBuffer[ 0 ][ j ] * scale ) + 1.0, -clip( outBuffer[ 1 ][ j ] * scale ) + 1.0 );
		paintMutex.unlock();
	}
	else
	{
		int drawFrom = -1;
		if ( !triggerChn ) //brak triggera
			drawFrom = samplesVisible;
		else
		{
			const int triggerChnIdx = triggerChn - 1;
			const float trgPos = triggerPos / scale;
			int to = samplesVisible * 3 / 2;

			if ( triggerHold > 0 )
				--triggerHold;

			for ( int i = samplesVisible / 2 + 1 ; i < to ; ++i )
			{
				bool trigger = fallingSlope ? ( getSample( triggerChnIdx, i-1 ) > trgPos && getSample( triggerChnIdx, i ) <= trgPos ) : ( getSample( triggerChnIdx, i-1 ) < trgPos && getSample( triggerChnIdx, i ) >= trgPos );
				if ( trigger )
				{
					drawFrom = i - samplesVisible / 2;
					triggerHold = qMin( round( ( double )Global::getSampleRate() / samplesVisible ), 1.0 / Global::getPeriod() );
					break;
				}
			}

			if ( drawFrom < 0 && !triggerHold )
				drawFrom = samplesVisible;
		}
		if ( drawFrom > -1 )
		{
			paintMutex.lock();
			paths.clear();
			paths.resize( outBuffer.count() );
			for ( int i = 0 ; i < paths.count() ; ++i )
			{
				paths[ i ].moveTo( 0.0, -clip( getSample( i, drawFrom ) * scale ) + 1.0 );
				for ( int j = 1 ; j < samplesVisible ; ++j )
				{
					if ( !interp )
						paths[ i ].lineTo( j, -clip( getSample( i, j+drawFrom-1 ) * scale ) + 1.0 );
					paths[ i ].lineTo( j, -clip( getSample( i, j+drawFrom ) * scale ) + 1.0 );
				}
			}
			paintMutex.unlock();
		}
	}
	QWidget::update();
}

void Scope::paintEvent( QPaintEvent * )
{
	paintMutex.lock();
	if ( !paths.isEmpty() )
	{
		QPainter p( this );
		if ( !xy )
			p.scale( ( width() - 1 ) / ( samplesVisible - 1.0 ), ( height() - 1 ) / 2.0 / paths.count() );
		else
		{
			const int s = width() > height() ? height() : width();
			p.translate( width() / 2 - s / 2, height() / 2 - s / 2 );
			p.scale( ( s - 1 ) / 2.0, ( s - 1 ) / 2.0 );
		}
		for ( int i = 0 ; i < paths.count() ; ++i )
		{
			if ( !xy )
			{
				p.setPen( QPen( QColor( 102, 51, 128 ), 0.0 ) );
				p.drawLine( QLineF( 0.0, 1.0, samplesVisible, 1.0 ) );

				if ( triggerChn == i+1 )
				{
					p.setPen( QPen( QColor( 50, 100, 150 ), 0.0, Qt::DotLine ) );
					p.drawLine( QLineF( 0.0, 1.0 - triggerPos, samplesVisible, 1.0 - triggerPos ) );
				}
			}
			p.setPen( QPen( QColor( 102, 179, 102 ), 0.0 ) );
			p.drawPath( paths.at( i ) );
			p.translate( 0.0, 2.0 );
		}
	}
	paintMutex.unlock();
}
void Scope::closeEvent( QCloseEvent *event )
{
	if ( cantClose )
	{
		event->ignore();
		return;
	}
	geo = saveGeometry();
	QWidget::closeEvent( event );
}
void Scope::showEvent( QShowEvent *event )
{
	if ( !geo.isEmpty() )
	{
		restoreGeometry( geo );
		geo.clear();
	}
	QWidget::showEvent( event );
}
void Scope::mouseMoveEvent( QMouseEvent *event )
{
	const int maxVal = 2000 / scale;
	QWidget::setToolTip( QString( "%1" ).arg( ( maxVal / 2 - ( event->pos().y() * inputsCount() * maxVal / height() ) % maxVal ) / 1000.0, 0, 'f', 3 ) );
	QWidget::mouseMoveEvent( event );
}

#include <QDoubleSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QLayout>
#include <QLabel>

ScopeUI::ScopeUI( Scope &block ) :
	AdditionalSettings( block ),
	block( block )
{
	QLabel *samplesVisibleL = new QLabel( "Ilość widocznych próbek: " );
	samplesVisibleL->setAlignment( Qt::AlignRight );

	QLabel *interpolationL = new QLabel( "Interpolacja: " );
	interpolationL->setAlignment( Qt::AlignRight );

	interpolationCB = new QComboBox;
	interpolationCB->addItems( QStringList() << "Zero order hold" << "Liniowa" );

	samplesVisibleB = new QSpinBox;
	samplesVisibleB->setRange( 2, Global::getSampleRate() * 2 );

	QLabel *scaleL = new QLabel( "Skala: " );
	scaleL->setAlignment( Qt::AlignRight );

	scaleB = new QDoubleSpinBox;
	scaleB->setDecimals( 3 );
	scaleB->setSingleStep( 0.01 );
	scaleB->setRange( 0.001, 100.0 );
	scaleB->setSuffix( "x" );

	QPushButton *applyB = new QPushButton( "&Zastosuj" );

	xyB = new QCheckBox( "XY" );

	triggerB = new QGroupBox( "Trigger" );
	triggerB->setCheckable( true );

	slopeCB = new QComboBox;
	slopeCB->addItems( QStringList() << "Zbocze narastające" << "Zbocze opadające" );

	QLabel *trgChnL = new QLabel( "Kanał: " );

	trgChnB = new QSpinBox;
	trgChnB->setMinimum( 1 );

	QLabel *trgPosL = new QLabel( "Pozycja: " );

	trgPosS = new QSlider( Qt::Horizontal );
	trgPosS->setRange( -50, 50 );

	QGridLayout *trgLayout = new QGridLayout( triggerB );
	trgLayout->addWidget( slopeCB, 0, 0, 1, 2 );
	trgLayout->addWidget( trgChnL, 1, 0, 1, 1 );
	trgLayout->addWidget( trgChnB, 1, 1, 1, 1 );
	trgLayout->addWidget( trgPosL, 2, 0, 1, 1 );
	trgLayout->addWidget( trgPosS, 2, 1, 1, 1 );
	trgLayout->setMargin( 3 );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( interpolationL, 0, 0, 1, 1 );
	layout->addWidget( interpolationCB, 0, 1, 1, 1 );
	layout->addWidget( samplesVisibleL, 1, 0, 1, 1 );
	layout->addWidget( samplesVisibleB, 1, 1, 1, 1 );
	layout->addWidget( scaleL, 2, 0, 1, 1 );
	layout->addWidget( scaleB, 2, 1, 1, 1 );
	layout->addWidget( applyB, 3, 0, 1, 2 );
	layout->addWidget( xyB, 4, 0, 1, 2 );
	layout->addWidget( triggerB, 5, 0, 1, 2 );
	layout->setMargin( 3 );

	connect( applyB, SIGNAL( clicked() ), this, SLOT( apply() ) );
}

void ScopeUI::prepare()
{
	interpolationCB->setCurrentIndex( block.interp );
	samplesVisibleB->setValue( block.samplesVisible );
	scaleB->setValue( block.scale );

	checkXY( block.inputsCount() );
	if ( xyB->isEnabled() )
	{
		xyB->setChecked( block.xy );
		interpolationCB->setDisabled( block.xy );
		samplesVisibleB->setDisabled( block.xy );
		triggerB->setDisabled( block.xy );
	}

	triggerB->setChecked( block.triggerChn );
	slopeCB->setCurrentIndex( block.fallingSlope );
	trgChnB->setMaximum( block.inputsCount() );
	trgChnB->setValue( block.triggerChn );
	trgPosS->setValue( block.triggerPos * 50 );


	connect( xyB, SIGNAL( toggled( bool ) ), this, SLOT( xyToggled( bool ) ) );
	connect( slopeCB, SIGNAL( currentIndexChanged( int ) ), SLOT( setTrigger() ) );
	connect( triggerB, SIGNAL( toggled( bool ) ), this, SLOT( setTrigger() ) );
	connect( trgChnB, SIGNAL( valueChanged( int ) ), this, SLOT( setTrigger() ) );
	connect( trgPosS, SIGNAL( valueChanged( int ) ), this, SLOT( setTrigger() ) );
}

void ScopeUI::inputsCountChanged( int num )
{
	checkXY( num );
	trgChnB->setMaximum( num );
}

void ScopeUI::xyToggled( bool b )
{
	interpolationCB->setDisabled( b );
	samplesVisibleB->setDisabled( b );
	triggerB->setDisabled( b );
	if ( block.xy != b )
	{
		block.execMutex.lock();
		block.drawMutex.lock();
		block.xy = b;
		block.setBuffers();
		block.paths.clear();
		block.drawMutex.unlock();
		block.execMutex.unlock();
		dynamic_cast< QWidget & >( block ).update();
	}
}
void ScopeUI::setTrigger()
{
	block.drawMutex.lock();
	block.fallingSlope = slopeCB->currentIndex();
	block.triggerChn = triggerB->isChecked() ? trgChnB->value() : 0;
	block.triggerPos = trgPosS->value() / 50.0f;
	block.drawMutex.unlock();
	dynamic_cast< QWidget & >( block ).update();
}
void ScopeUI::apply()
{
	block.execMutex.lock();
	block.drawMutex.lock();
	block.interp = interpolationCB->currentIndex();
	if ( block.samplesVisible != samplesVisibleB->value() && !block.xy )
	{
		block.samplesVisible = samplesVisibleB->value();
		block.setBuffers();
	}
	block.scale = scaleB->value();
	block.drawMutex.unlock();
	block.execMutex.unlock();
}

void ScopeUI::checkXY( int numInputs )
{
	xyB->setEnabled( numInputs == 2 );
	if ( !xyB->isEnabled() )
		xyB->setChecked( false );
}
