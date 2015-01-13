#include "Image.hpp"
#include "Global.hpp"
#include "Array.hpp"

extern "C"
{
	#include <libavcodec/avfft.h>
	#include <libavutil/mem.h>
}

#include <QDebug>

Image::Image() :
	Block( "Image", "Generuje obraz", 0, 1, SOURCE ),
	fftNbits( 10 )
{}

bool Image::start()
{
	settings->setRunMode( true );
	if ( outputsCount() == 1 )
		return fft_gen_img();
	else if ( outputsCount() == 2 )
		return xy_gen_img();
	return false;
}
void Image::exec( Array< Sample > &samples )
{
	mutex.lock();
	if ( rdy_samples.notEmpty() )
		for ( int i = 0 ; i < outputsCount() ; ++i )
			samples += ( Sample ){ getTarget( i ), rdy_samples.get() };
	else
		samples += ( Sample ){ getTarget( 0 ), 0.0f/0.0f };
	mutex.unlock();
}
void Image::stop()
{
	settings->setRunMode( false );
	rdy_samples.resize( 0 );
}

Block *Image::createInstance()
{
	Image *block = new Image;
	block->settings = new Settings( *block, false, 0, 0, true, 1, 2, false, new ImageUI( *block ) );
	block->setLabel();
	return block;
}

void Image::serialize( QDataStream &ds ) const
{
	ds << fftNbits << file;
}
void Image::deSerialize( QDataStream &ds )
{
	ds >> fftNbits >> file;
}

void Image::outputsCountChanged( int num )
{
	Q_UNUSED( num )
	setLabel();
	qobject_cast< ImageUI * >( settings->getAdditionalSettings() )->itemsVisible();
}

bool Image::fft_gen_img()
{
	if ( fftNbits < 2 || fftNbits > 16 )
		fftNbits = 10;
	QImage img( QString( file.data() ) );
	if ( img.width() < 1 || img.height() < 1 )
		rdy_samples.resize( 0 );
	else
	{
		const int fftSize = 1 << fftNbits;

		FFTContext *fftOut = av_fft_init( fftNbits, true );
		FFTComplex *fftCplx = ( FFTComplex * )av_malloc( fftSize * sizeof( FFTComplex ) );

		img = img.scaledToHeight( fftSize / 2, Qt::SmoothTransformation );
		quint32 *pixels = ( quint32 * )img.bits();
		rdy_samples.resize( img.width() * fftSize );
		for ( int x = 0 ; x < img.width() ; ++x )
		{
			for ( int y = 0 ; y < img.height() ; ++y )
			{
				QColor c = QColor::fromRgba( pixels[ ( img.height()-1-y ) * img.width() + x ] );
				fftCplx[ y ].re = fftCplx[ fftSize-1-y ].re = ( c.red() + c.green() + c.blue() ) / 765.0f / fftSize;
				fftCplx[ y ].im = fftCplx[ fftSize-1-y ].im = 0.0f;
			}
			av_fft_permute( fftOut, fftCplx );
			av_fft_calc( fftOut, fftCplx );
			for ( int i = 0 ; i < fftSize ; ++i )
				rdy_samples += fftCplx[ i ].re;
		}

		av_free( fftCplx );
		av_fft_end( fftOut );

		return true;
	}
	return false;
}
bool Image::xy_gen_img()
{
	QImage img( QString( file.data() ) );
	if ( img.width() < 1 || img.height() < 1 )
		rdy_samples.resize( 0 );
	else
	{
		int num_pixels = 0, posX = -1, posY = -1;
		img = img.convertToFormat( QImage::Format_Mono );

		/* Liczenie ilości pikseli oraz szukanie pierwszego pixela */
		for ( int y = 0 ; y < img.height() ; ++y )
			for ( int x = 0 ; x < img.width() ; ++x )
				if ( QColor( img.pixel( x, y ) ).black() )
				{
					++num_pixels;
					if ( posX < 0 || posY < 0 )
					{
						posX = x;
						posY = y;
						img.setPixel( x, y, 0 );
						--num_pixels;
					}
				}

		/* Jeżeli jest choć jeden pixel */
		if ( posX >= 0 && posY >= 0 )
		{
			rdy_samples.resize( ( num_pixels + 1 ) * 2 );
			rdy_samples += posX * 2 / ( float )img.width() - 1.0f;
			rdy_samples += 1.0f - posY * 2 / ( float )img.height();
		}

		while ( num_pixels > 0 )
		{
			int x = posX, y = posY;

			bool moveX = true;
			int direction[ 4 ] = { 1, -1, -1, 1 };
			int d = 0, steps = 1, pos = 0;

			/* Wyszukiwanie najbliższego pixela */
			do
			{
				if ( moveX )
					x += direction[ d ];
				else
					y += direction[ d ];

				if ( ++pos == steps )
				{
					d = ( d + 1 ) % 4;
					moveX = !moveX;
					if ( moveX )
						++steps;
					pos = 0;
				}

				posX = x;
				if ( posX < 0 )
					posX = 0;
				else if ( posX >= img.width() )
					posX = img.width() - 1;

				posY = y;
				if ( posY < 0 )
					posY = 0;
				else if ( posY >= img.height() )
					posY = img.height() - 1;
			} while ( !QColor( img.pixel( posX, posY ) ).black() );

			rdy_samples += posX * 2 / ( float )img.width() - 1.0f;
			rdy_samples += 1.0f - posY * 2 / ( float )img.height();

			img.setPixel( posX, posY, 0 );

			--num_pixels;
		}

		return rdy_samples.notEmpty();
	}
	return false;
}
void Image::setLabel()
{
	if ( outputsCount() == 1 )
		label = "Spectogram\nimage";
	else if ( outputsCount() == 2 )
		label = "XY\nimage";
	update();
}

#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLayout>
#include <QLabel>

ImageUI::ImageUI( Image &block ) :
	AdditionalSettings( block ),
	block( block )
{
	fileE = new QLineEdit;
	fileE->setPlaceholderText( "Ścieżka do pliku" );

	QToolButton *browseB = new QToolButton;
	browseB->setToolTip( "Przeglądaj" );
	browseB->setText( "..." );

	fftSizeL = new QLabel( "Rozmiar tablicy FFT: " );

	fftSizeB = new QComboBox;
	fftSizeB->addItems( QStringList() << "4" << "8" << "16" << "32" << "64" << "128" << "256" << "512" << "1024" << "2048" << "4096" << "8192" << "16384" << "32768" << "65536" );

	browseB->setSizePolicy( fftSizeB->sizePolicy() );

	QPushButton *applyB = new QPushButton( "&Zastosuj" );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( fileE, 0, 0, 1, 1 );
	layout->addWidget( browseB, 0, 1, 1, 1 );
	layout->addWidget( fftSizeL, 1, 0, 1, 1 );
	layout->addWidget( fftSizeB, 1, 1, 1, 1 );
	layout->addWidget( applyB, 2, 0, 1, 2 );
	layout->setMargin( 3 );

	connect( browseB, SIGNAL( clicked() ), this, SLOT( browseFile() ) );
	connect( applyB, SIGNAL( clicked() ), this, SLOT( apply() ) );
}

void ImageUI::prepare()
{
	fileE->setText( block.file );
	fftSizeB->setCurrentIndex( block.fftNbits - 2 );
	itemsVisible();
}

void ImageUI::itemsVisible()
{
	fftSizeL->setVisible( block.outputsCount() == 1 );
	fftSizeB->setVisible( fftSizeL->isVisible() );
}

void ImageUI::browseFile()
{
	QString newFile = QFileDialog::getOpenFileName( this, "Wybierz plik obrazka", fileE->text(), QString(), NULL, Global::getNativeFileDialogFlag() );
	if ( !newFile.isEmpty() )
		fileE->setText( newFile );
}
void ImageUI::apply()
{
	block.fftNbits = fftSizeB->currentIndex() + 2;
	block.file = fileE->text().toUtf8();
	block.mutex.lock();
	if ( block.outputsCount() == 1 )
		block.fft_gen_img();
	else if ( block.outputsCount() == 2 )
		block.xy_gen_img();
	block.mutex.unlock();
}
