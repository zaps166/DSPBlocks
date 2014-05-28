#include "Scope.hpp"

#include <QCloseEvent>
#include <QPainter>
#include <QTimer>
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
	samplesVisible( 1024 ),
	scale( 1.0f ),
	xy( false ),
	fallingSlope( false ), triggerChn( 0 ), triggerPos( 0.2f ),
	cantClose( false )
{
	setStyle( &style );
	setWindowTitle( "Oscyloskop" );
	setPalette( Qt::black );
}

bool Scope::start()
{
	settings->setRunMode( true );
	cantClose = true;
	QWidget::show();

	buffer.resize( inputsCount() );
	setBuffer();

	return true;
}
void Scope::setSample( int input, float sample )
{
	mutex.lock();
	buffer[ input ][ xy ? buffPos : ( buffPos + samplesVisible ) ] = sample;
	mutex.unlock();
}
void Scope::exec( Array< Sample > & )
{
	mutex.lock();
	if ( xy && ++buffPos == buffer[ 0 ].count() )
	{
		paths.clear();
		paths.resize( 1 );

		paths[ 0 ].moveTo( clip( buffer[ 0 ][ 0 ] * scale ) + 1.0, -clip( buffer[ 1 ][ 0 ] * scale ) + 1.0 );
		for ( int j = 1 ; j < buffPos ; ++j )
			paths[ 0 ].lineTo( clip( buffer[ 0 ][ j ] * scale ) + 1.0, -clip( buffer[ 1 ][ j ] * scale ) + 1.0 );

		QTimer::singleShot( 0, this, SLOT( update() ) );
		buffPos = 0;
	}
	else if ( !xy && ++buffPos == samplesVisible )
	{
		int from = -1;
		if ( !triggerChn ) //bez triggera
			from = samplesVisible;
		else
		{
			const QVector< float > &trgBuff = buffer[ triggerChn-1 ];
			float trgPos = triggerPos / scale;
			int to = samplesVisible * 3 / 2;
			for ( int i = samplesVisible / 2 + 1 ; i < to ; ++i )
			{
				bool trigger = fallingSlope ? ( trgBuff[ i-1 ] > trgPos && trgBuff[ i ] <= trgPos ) : ( trgBuff[ i-1 ] < trgPos && trgBuff[ i ] >= trgPos );
				if ( trigger )
				{
					from = i - samplesVisible / 2;
					triggerHold = qMax( samplesVisible, getSampleRate() );
					break;
				}
				else if ( triggerHold > 0 )
					--triggerHold;
			}
			if ( from < 0 && !triggerHold )
				from = samplesVisible;
		}
		if ( from > -1 )
			setSamples( from );
		for ( int i = 0 ; i < buffer.count() ; ++i )
			memcpy( buffer[ i ].data(), buffer[ i ].data() + samplesVisible, sizeof( float ) * samplesVisible );
		buffPos = 0;
	}
	mutex.unlock();
}
void Scope::stop()
{
	settings->setRunMode( false );
	cantClose = false;
	buffer.clear();
}

Block *Scope::createInstance()
{
	Scope *block = new Scope;
	block->settings = new Settings( *block, true, 1, maxIO, false, 0, 0, false, new ScopeUI( *block ) );
	return block;
}

void Scope::serialize( QDataStream &ds ) const
{
	ds << xy << samplesVisible << scale << fallingSlope << triggerChn << triggerPos;
}
void Scope::deSerialize( QDataStream &ds )
{
	ds >> xy >> samplesVisible >> scale >> fallingSlope >> triggerChn >> triggerPos;
}

void Scope::inputsCountChanged( int num )
{
	qobject_cast< ScopeUI * >( settings->getAdditionalSettings() )->inputsCountChanged( num );
}

void Scope::setBuffer()
{
	for ( int i = 0 ; i < buffer.count() ; ++i )
	{
		buffer[ i ].clear();
		buffer[ i ].resize( xy ? ( getSampleRate() * getPeriod() ) : ( samplesVisible * 2 ) );
	}
	buffPos = triggerHold = 0;
}
void Scope::setSamples( int from )
{
	paths.clear();
	paths.resize( buffer.count() );
	for ( int i = 0 ; i < paths.count() ; ++i )
	{
		paths[ i ].moveTo( 0.0, -clip( buffer[ i ][ from ] * scale ) + 1.0 );
		for ( int j = 1 ; j < samplesVisible ; ++j )
			paths[ i ].lineTo( j, -clip( buffer[ i ][ j+from ] * scale ) + 1.0 );
	}
	QTimer::singleShot( 0, this, SLOT( update() ) );
}

void Scope::paintEvent( QPaintEvent * )
{
	mutex.lock();
	if ( !paths.isEmpty() )
	{
		QPainter p( this );
		if ( !xy )
			p.scale( ( width() - 1 ) / ( paths[ 0 ].elementCount() - 1.0 ), ( height() - 1 ) / 2.0 / paths.count() );
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
				p.drawLine( QLineF( 0.0, 1.0, paths[ i ].elementCount(), 1.0 ) );

				if ( triggerChn == i+1 )
				{
					p.setPen( QPen( QColor( 50, 100, 150 ), 0.0, Qt::DotLine ) );
					p.drawLine( QLineF( 0.0, 1.0 - triggerPos, paths[ i ].elementCount(), 1.0 - triggerPos ) );
				}
			}
			p.setPen( QPen( QColor( 102, 179, 102 ), 0.0 ) );
			p.drawPath( paths[ i ] );
			p.translate( 0.0, 2.0 );
		}
	}
	mutex.unlock();
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

	samplesVisibleB = new QSpinBox;
	samplesVisibleB->setRange( 2, Block::getSampleRate() * 2 );

	QLabel *scaleL = new QLabel( "Skala: " );
	scaleL->setAlignment( Qt::AlignRight );

	scaleB = new QDoubleSpinBox;
	scaleB->setDecimals( 3 );
	scaleB->setSingleStep( 0.01 );
	scaleB->setRange( 0.001, 100.0 );
	scaleB->setSuffix( "x" );

	xyB = new QCheckBox( "XY" );

	triggerB = new QGroupBox( "Trigger" );
	triggerB->setCheckable( true );

	slopeCB = new QComboBox;
	slopeCB->addItems( QStringList() << "Zbocze narastające" << "Zbocze opadające" );

	QLabel *trgChnL = new QLabel( "Kanał: " );

	trgChnB = new QSpinBox;
	trgChnB->setMinimum( 1 );

	QLabel *trgPosL = new QLabel( "Próg: " );

	trgPosS = new QSlider( Qt::Horizontal );
	trgPosS->setRange( -50, 50 );

	QGridLayout *trgLayout = new QGridLayout( triggerB );
	trgLayout->addWidget( slopeCB, 0, 0, 1, 2 );
	trgLayout->addWidget( trgChnL, 1, 0, 1, 1 );
	trgLayout->addWidget( trgChnB, 1, 1, 1, 1 );
	trgLayout->addWidget( trgPosL, 2, 0, 1, 1 );
	trgLayout->addWidget( trgPosS, 2, 1, 1, 1 );
	trgLayout->setMargin( 3 );

	QPushButton *applyB = new QPushButton( "&Zastosuj" );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( samplesVisibleL, 0, 0, 1, 1 );
	layout->addWidget( samplesVisibleB, 0, 1, 1, 1 );
	layout->addWidget( scaleL, 1, 0, 1, 1 );
	layout->addWidget( scaleB, 1, 1, 1, 1 );
	layout->addWidget( xyB, 2, 1, 1, 1 );
	layout->addWidget( triggerB, 3, 0, 1, 2 );
	layout->addWidget( applyB, 4, 0, 1, 2 );
	layout->setMargin( 3 );

	connect( applyB, SIGNAL( clicked() ), this, SLOT( apply() ) );
}

void ScopeUI::prepare()
{
	samplesVisibleB->setValue( block.samplesVisible );
	scaleB->setValue( block.scale );

	checkXY( block.inputsCount() );
	if ( xyB->isEnabled() )
	{
		xyB->setChecked( block.xy );
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
	samplesVisibleB->setDisabled( b );
	triggerB->setDisabled( b );
	if ( block.xy != b )
	{
		block.mutex.lock();
		block.xy = b;
		block.setBuffer();
		block.paths.clear();
		block.mutex.unlock();
		dynamic_cast< QWidget & >( block ).update();
	}
}
void ScopeUI::setTrigger()
{
	block.mutex.lock();
	block.fallingSlope = slopeCB->currentIndex();
	block.triggerChn = triggerB->isChecked() ? trgChnB->value() : 0;
	block.triggerPos = trgPosS->value() / 50.0f;
	block.mutex.unlock();
	dynamic_cast< QWidget & >( block ).update();
}
void ScopeUI::apply()
{
	block.mutex.lock();
	if ( block.samplesVisible != samplesVisibleB->value() && !block.xy )
	{
		block.samplesVisible = samplesVisibleB->value();
		block.setBuffer();
	}
	block.scale = scaleB->value();
	block.mutex.unlock();
}

void ScopeUI::checkXY( int numInputs )
{
	xyB->setEnabled( numInputs == 2 );
	if ( !xyB->isEnabled() )
		xyB->setChecked( false );
}
