#include "Spectrum.hpp"

#include <QCloseEvent>
#include <QPainter>
#include <QDebug>
#include <QTimer>

extern "C"
{
	#include <libavcodec/avfft.h>
	#include <libavutil/mem.h>
}
#include <math.h>

Spectrum::Spectrum() :
	Block( "Spectrum", "Widmo amplitudowe", 1, 1, SINK ),
	fftCtx( NULL ), fftCplx( NULL ),
	spectogram( false ),
	fftSize( 1024 ), numSpectrums( 500 ),
	cantClose( false ),
	spectrumScale( 1.0f )
{
	setStyle( &style );
	setWindowTitle( "Analiza widmowa" );
	setPalette( Qt::black );
	setMouseTracking( true );

	linearGrad.setStart( 0.0, 0.0 );
	linearGrad.setFinalStop( 1.0, 0.0 );
	linearGrad.setColorAt( 0.0, Qt::red );
	linearGrad.setColorAt( 0.1, Qt::yellow );
	linearGrad.setColorAt( 0.4, Qt::green );
	linearGrad.setColorAt( 0.9, Qt::blue );
}

bool Spectrum::start()
{
	settings->setRunMode( true );
	cantClose = true;
	QWidget::show();
	setFFT();
	return true;
}
void Spectrum::setSample( int input, float sample )
{
	mutex.lock();
	fftCplx[ input ][ pos ] = ( FFTComplex ){ sample, 0.0f };
	mutex.unlock();
}
void Spectrum::exec( Array< Sample > & )
{
	mutex.lock();
	if ( ++pos == fftSize )
	{
		for ( int i = 0 ; i < inputsCount() ; ++i )
		{
			av_fft_permute( fftCtx, fftCplx[ i ] );
			av_fft_calc( fftCtx, fftCplx[ i ] );
		}

		int spectrumSize = fftSize / 2;
		if ( spectogram )
		{
			quint32 *pixels = ( quint32 * )spectogramImg.bits();
			int numSpectrumMinus1 = numSpectrums-1;
			for ( int line = spectrumSize * inputsCount() * numSpectrums - numSpectrums ; line >= 0 ; line -= numSpectrums )
				memmove( pixels + line, pixels + 1 + line, numSpectrumMinus1 * sizeof *pixels );
			int y = spectrumSize * inputsCount();
			for ( int i = inputsCount()-1 ; i >= 0 ; --i )
				for ( int j = 0 ; j < spectrumSize ; ++j )
				{
					quint32 val = sqrt( fftCplx[ i ][ j ].re * fftCplx[ i ][ j ].re + fftCplx[ i ][ j ].im * fftCplx[ i ][ j ].im ) * spectrumScale * 255 / spectrumSize;
					if ( val > 255 )
						val = 255;
					pixels[ --y * numSpectrums + numSpectrumMinus1 ] = QColor( val, val, val ).rgba();
				}
		}
		else for ( int i = 0 ; i < inputsCount() ; ++i )
			for ( int j = 0 ; j < spectrumSize ; ++j )
				spectrum[ i ][ j ] = sqrt( fftCplx[ i ][ j ].re * fftCplx[ i ][ j ].re + fftCplx[ i ][ j ].im * fftCplx[ i ][ j ].im ) * spectrumScale / spectrumSize;

		QTimer::singleShot( 0, this, SLOT( update() ) );

		pos = 0;
	}
	mutex.unlock();
}
void Spectrum::stop()
{
	settings->setRunMode( false );
	cantClose = false;
	freeFFT();
}

Block *Spectrum::createInstance()
{
	Spectrum *block = new Spectrum;
	block->settings = new Settings( *block, true, 1, maxIO, false, 0, 0, false, new SpectrumUI( *block ) );
	return block;
}

void Spectrum::setFFT()
{
	freeFFT();

	fftCtx = av_fft_init( log2( fftSize ), false );
	fftCplx = new FFTComplex *[ inputsCount() ];
	for ( int i = 0 ; i < inputsCount() ; ++i )
		fftCplx[ i ] = ( FFTComplex * )av_mallocz( fftSize * sizeof( FFTComplex ) );

	spectrum.clear();

	if ( spectogram )
	{
		QSize imgSize( numSpectrums, fftSize / 2 * inputsCount() );
		if ( spectogramImg.size() != imgSize )
		{
			spectogramImg = QImage( imgSize, QImage::Format_RGB32 );
			spectogramImg.fill( Qt::black );
		}
	}
	else
	{
		spectogramImg = QImage();
		spectrum.resize( inputsCount() );
		for ( int i = 0 ; i < inputsCount() ; ++i )
			spectrum[ i ].resize( fftSize / 2 );
	}

	pos = 0;
}
void Spectrum::freeFFT()
{
	if ( fftCtx )
	{
		av_fft_end( fftCtx );
		fftCtx = NULL;
	}
	if ( fftCplx )
	{
		for ( int i = 0 ; i < inputsCount() ; ++i )
			av_free( fftCplx[ i ] );
		delete[] fftCplx;
		fftCplx = NULL;
	}
}
void Spectrum::setLabel()
{
	if ( spectogram )
	{
		label = "Spectogram";
		setWindowTitle( "Spektogram" );
	}
	else
	{
		label = getName();
		setWindowTitle( "Analiza widmowa" );
	}
	QGraphicsItem::update();
}

void Spectrum::serialize( QDataStream &ds ) const
{
	ds << fftSize << spectogram << numSpectrums << spectrumScale;
}
void Spectrum::deSerialize( QDataStream &ds )
{
	ds >> fftSize >> spectogram >> numSpectrums >> spectrumScale;
	setLabel();
}

void Spectrum::paintEvent( QPaintEvent * )
{
	mutex.lock();
	QPainter p( this );
	if ( spectogram )
		p.drawImage( 0, 0, spectogramImg.scaled( width(), height() ) );
	else
	{
		const int spectrumSize = fftSize / 2;
		const qreal rectS = 1.0 / spectrumSize;
		p.scale( width(), height() / ( qreal )inputsCount() );
		for ( int i = 0 ; i < inputsCount() ; ++i )
		{
			for ( int j = 0 ; j < spectrumSize ; ++j )
			{
				const qreal x = ( qreal )j / spectrumSize;
				p.fillRect( QRectF( x, 1.0 - spectrum[ i ][ j ], rectS, spectrum[ i ][ j ] ), linearGrad );
			}
			p.translate( 0.0, 1.0 );
		}
	}
	mutex.unlock();
}
void Spectrum::closeEvent( QCloseEvent *event )
{
	if ( cantClose )
	{
		event->ignore();
		return;
	}
	geo = saveGeometry();
	QWidget::closeEvent( event );
}
void Spectrum::showEvent( QShowEvent *event )
{
	if ( !geo.isEmpty() )
	{
		restoreGeometry( geo );
		geo.clear();
	}
	QWidget::showEvent( event );
}
void Spectrum::mouseMoveEvent( QMouseEvent *event )
{
	int maxFreq = Block::getSampleRate() / 2;
	QString pointedFreqStr;
	int pointedFreq;
	if ( spectogram )
		pointedFreq = maxFreq - ( ( event->pos().y() * inputsCount() * maxFreq / height() ) % maxFreq );
	else
		pointedFreq = event->pos().x() * maxFreq / width();
	if ( pointedFreq >= 1000 )
		pointedFreqStr = QString( "%1 kHz" ).arg( pointedFreq / 1000.0, 0, 'f', 2 );
	else
		pointedFreqStr = QString( "%1 Hz" ).arg( pointedFreq );
	QWidget::setToolTip( pointedFreqStr );
	QWidget::mouseMoveEvent( event );
}

#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLayout>
#include <QLabel>

SpectrumUI::SpectrumUI( Spectrum &block ) :
	AdditionalSettings( block ),
	block( block )
{
	QLabel *fftSizeL = new QLabel( "Rozmiar tablicy FFT: " );

	fftSizeB = new QComboBox;
	fftSizeB->addItems( QStringList() << "4" << "8" << "16" << "32" << "64" << "128" << "256" << "512" << "1024" << "2048" << "4096" << "8192" << "16384" << "32768" << "65536" );

	spectogramB = new QCheckBox( "Spektogram" );

	numSpectrumsB = new QSpinBox;
	numSpectrumsB->setPrefix( "Ilość kawałków: " );
	numSpectrumsB->setRange( 1, 9999 );

	scaleB = new QDoubleSpinBox;
	scaleB->setDecimals( 1 );
	scaleB->setRange( 0.1, 1000.0 );
	scaleB->setPrefix( "Skala: " );
	scaleB->setSuffix( "x" );

	QPushButton *applyB = new QPushButton( "&Zastosuj" );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( fftSizeL, 0, 0, 1, 1 );
	layout->addWidget( fftSizeB, 0, 1, 1, 1 );
	layout->addWidget( spectogramB, 1, 0, 1, 2 );
	layout->addWidget( numSpectrumsB, 2, 0, 1, 2 );
	layout->addWidget( scaleB, 3, 0, 1, 2 );
	layout->addWidget( applyB, 4, 0, 1, 2 );
	layout->setMargin( 3 );

	connect( applyB, SIGNAL( clicked() ), this, SLOT( setValue() ) );
}

void SpectrumUI::prepare()
{
	fftSizeB->setCurrentIndex( log2( block.fftSize ) - 2 );
	spectogramB->setChecked( block.spectogram );
	numSpectrumsB->setValue( block.numSpectrums );
	scaleB->setValue( block.spectrumScale );

	numSpectrumsB->setEnabled( spectogramB->isChecked() );

	connect( spectogramB, SIGNAL( toggled( bool ) ), numSpectrumsB, SLOT( setEnabled( bool ) ) );
}

void SpectrumUI::setValue()
{
	int fftSize = fftSizeB->currentText().toInt();
	if ( block.fftSize != fftSize || block.spectogram != spectogramB->isChecked() || block.numSpectrums != numSpectrumsB->value() || block.spectrumScale != scaleB->value() )
	{
		block.mutex.lock();
		block.fftSize = fftSize;
		block.spectogram = spectogramB->isChecked();
		block.numSpectrums = numSpectrumsB->value();
		block.spectrumScale = scaleB->value();
		if ( dynamic_cast< QWidget & >( block ).isVisible() )
			block.setFFT();
		block.mutex.unlock();
		block.setLabel();
	}
}
