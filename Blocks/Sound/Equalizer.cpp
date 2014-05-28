#include "Equalizer.hpp"
#include "Array.hpp"

extern "C"
{
	#include <libavcodec/avfft.h>
	#include <libavutil/mem.h>
}

#include <QDebug>

static inline void fft_calc( FFTContext *fft_ctx, FFTComplex *cplx )
{
	av_fft_permute( fft_ctx, cplx );
	av_fft_calc( fft_ctx, cplx );
}

Equalizer::Equalizer() :
	Block( "Equalizer", "Korektor graficzny", 1, 1, PROCESSING ),
	fftNbits( 10 ),
	fftIn( NULL ), fftOut( NULL ),
	fftCplx( NULL )
{}

bool Equalizer::start()
{
	if ( inputsCount() != outputsCount() )
		return false;
	settings->setRunMode( true );
	fft_start();
	return true;
}
void Equalizer::setSample( int input, float sample )
{
	in_buffer[ input ][ pos + fftSize_2 ] = sample;
}
void Equalizer::exec( Array< Sample > &samples )
{
	mutex.lock();
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), rdy_samples[ i ][ pos ] };
	if ( ++pos == fftSize_2 )
	{
		for ( int i = 0 ; i < outputsCount() ; ++i )
		{
			for ( int j = 0 ; j < fftSize ; ++j )
				fftCplx[ j ] = ( FFTComplex ){ in_buffer[ i ][ j ], 0.0f };
			memcpy( in_buffer[ i ].data(), in_buffer[ i ].data() + fftSize_2, sizeof( float ) * fftSize_2 );
			fft_calc( fftIn, fftCplx );
			for ( int j = 0, j2 = fftSize - 1 ; j < fftSize_2 ; ++j, --j2 )
			{
				fftCplx[ j  ].re *= f[ j ];
				fftCplx[ j  ].im *= f[ j ];
				fftCplx[ j2 ].re *= f[ j ];
				fftCplx[ j2 ].im *= f[ j ];
			}
			fft_calc( fftOut, fftCplx );
			for ( int j = 0, j2 = fftSize_2 ; j < fftSize_2 ; ++j, ++j2 )
			{
				rdy_samples[ i ][ j  ] = fftCplx[ j  ].re / fftSize * wind_f[ j  ] + rdy_samples[ i ][ j2 ];
				rdy_samples[ i ][ j2 ] = fftCplx[ j2 ].re / fftSize * wind_f[ j2 ];
			}
		}
		pos = 0;
	}
	mutex.unlock();
}
void Equalizer::stop()
{
	settings->setRunMode( false );
	fft_free();
}

Block *Equalizer::createInstance()
{
	Equalizer *block = new Equalizer;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, true, new EqualizerUI( *block ) );
	return block;
}

void Equalizer::serialize( QDataStream &ds ) const
{
	ds << fftNbits;
}
void Equalizer::deSerialize( QDataStream &ds )
{
	ds >> fftNbits;
}

void Equalizer::fft_start()
{
	if ( fftNbits < 4 || fftNbits > 16 )
		fftNbits = 10;
	fftSize   = 1 << fftNbits;
	fftSize_2 = fftSize >> 1;
	pos = 0;

	fftIn  = av_fft_init( fftNbits, false );
	fftOut = av_fft_init( fftNbits, true );
	fftCplx = ( FFTComplex * )av_malloc( fftSize * sizeof( FFTComplex ) );

	wind_f.resize( fftSize );
	f.resize( fftSize );
	for ( int i = 0 ; i < fftSize ; ++i )
	{
		wind_f[ i ] = 0.5f - 0.5f * cos( 2.0 * M_PI * i / ( fftSize - 1 ) );
		f[ i ] = cos( 2.0 * M_PI * i * 10 / fftSize ) / 2.0f + 0.5f;
	}

	in_buffer.resize( inputsCount() );
	rdy_samples.resize( inputsCount() );
	for ( int i = 0 ; i < inputsCount() ; ++i )
	{
		in_buffer[ i ].resize( fftSize );
		rdy_samples[ i ].resize( fftSize );
	}
}
void Equalizer::fft_free()
{
	rdy_samples.clear();
	in_buffer.clear();
	wind_f.clear();
	f.clear();

	av_free( fftCplx );
	av_fft_end( fftIn );
	av_fft_end( fftOut );
	fftIn = fftOut = NULL;
	fftCplx = NULL;
}

#include <QPushButton>
#include <QComboBox>
#include <QLayout>
#include <QLabel>

EqualizerUI::EqualizerUI( Equalizer &block ) :
	AdditionalSettings( block ),
	block( block )
{
	QLabel *fftSizeL = new QLabel( "Rozmiar tablicy FFT: " );

	fftSizeB = new QComboBox;
	fftSizeB->addItems( QStringList() << "4" << "8" << "16" << "32" << "64" << "128" << "256" << "512" << "1024" << "2048" << "4096" << "8192" << "16384" << "32768" << "65536" );

	QPushButton *applyB = new QPushButton( "&Zastosuj" );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( fftSizeL, 0, 0, 1, 1 );
	layout->addWidget( fftSizeB, 0, 1, 1, 1 );
	layout->addWidget( applyB, 1, 0, 1, 2 );
	layout->setMargin( 3 );

	connect( applyB, SIGNAL( clicked() ), this, SLOT( setValue() ) );
}

void EqualizerUI::prepare()
{
	fftSizeB->setCurrentIndex( block.fftNbits - 2 );
}

void EqualizerUI::setValue()
{
	int fftNbits = fftSizeB->currentIndex() + 2;
	if ( block.fftNbits != fftNbits )
	{
		block.fftNbits = fftNbits;
		if ( block.fftCplx )
		{
			block.mutex.lock();
			block.fft_free();
			block.fft_start();
			block.mutex.unlock();
		}
	}
}
