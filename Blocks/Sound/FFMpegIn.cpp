#include "FFMpegIn.hpp"
#include "Array.hpp"

#include <QDebug>

extern "C"
{
	#include <libavformat/avformat.h>
	#include <libswresample/swresample.h>
	#include <libavutil/opt.h>
}

FFMpegIn::FFMpegIn() :
	Block( "FFMpeg input", "Odtwarza pliki dźwiękowe", 0, 1, SOURCE ),
	fmtCtx( NULL ), aStream( NULL ), aCodec( NULL ), swr( NULL ),
	frame( ( AVFrame * )av_mallocz( sizeof *frame ) ),
	srate( 0 ),
	loop( false )
{}
FFMpegIn::~FFMpegIn()
{
	av_free( frame );
}

bool FFMpegIn::start()
{
	srate = getSampleRate();
	settings->setRunMode( true );
	return ffmpegStart();
}
void FFMpegIn::exec( Array< Sample > &samples )
{
	mutex.lock();
	if ( fmtCtx && !outBufferSize )
	{
		AVPacket pkt;
		pkt.data = NULL;
		int e;
		while ( ( e = av_read_frame( fmtCtx, &pkt ) ) >= 0 )
		{
			if ( pkt.stream_index == aStream->index )
			{
				AVPacket pkt_tmp = pkt;
				int ok = 0, bread;
				outBufferSize = 0;
				while ( ( bread = avcodec_decode_audio4( aCodec, frame, &ok, &pkt_tmp ) ) >= 0 && ok )
				{
					if ( aCodec->channels == channels )
					{
						const int out_size = ceil( frame->nb_samples * ( float )srate / ( float )aCodec->sample_rate );
						int buffer_size = outBufferSize + out_size * outputsCount();
						if ( outBuffer.size() < buffer_size )
							outBuffer.resize( buffer_size );
						quint8 *out[] = { ( quint8 * )( outBuffer.data() + outBufferSize ) };
						if ( ( buffer_size = swr_convert( swr, out, out_size, ( const quint8 ** )frame->extended_data, frame->nb_samples ) ) > 0 )
							outBufferSize += buffer_size * outputsCount();
					}
					if ( duration > 0 )
					{
						double currentTime = av_frame_get_best_effort_timestamp( frame ) * av_q2d( aStream->time_base );
						if ( currentTime - lastTime >= 1.0 )
						{
							lastTime = ( int )currentTime;
							emit dynamic_cast< FFMpegInUI * >( settings->getAdditionalSettings() )->setTime( lastTime );
						}
					}
					pkt_tmp.data += bread;
					pkt_tmp.size -= bread;
				}
				outBufferPos = 0;
			}
			av_free_packet( &pkt );
			if ( outBufferSize )
				break;
		}
		if ( e == AVERROR_EOF && loop && !av_seek_frame( fmtCtx, -1, 0, AVSEEK_FLAG_BACKWARD ) )
		{
			avcodec_flush_buffers( aCodec );
			lastTime = 0;
		}
	}
	if ( outBufferSize )
	{
		for ( int i = 0 ; i < outputsCount() ; ++i )
			samples += ( Sample ){ getTarget( i ), outBuffer[ outBufferPos + i ] };
		outBufferPos += outputsCount();
		outBufferSize -= outputsCount();
	}
	else for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), 0.0f };
	mutex.unlock();
}
void FFMpegIn::stop()
{
	settings->setRunMode( false );
	if ( fmtCtx )
		ffmpegStop();
	srate = 0;
}

Block *FFMpegIn::createInstance()
{
	FFMpegIn *block = new FFMpegIn;
	block->settings = new Settings( *block, false, 0, 0, true, 1, maxIO, false, new FFMpegInUI( *block ) );
	block->settings->resize( 500, 0 );
	return block;
}

void FFMpegIn::serialize( QDataStream &ds ) const
{
	ds << file << loop;
}
void FFMpegIn::deSerialize( QDataStream &ds )
{
	ds >> file >> loop;
}

bool FFMpegIn::ffmpegStart()
{
	if ( avformat_open_input( &fmtCtx, file, NULL, NULL ) == 0 && avformat_find_stream_info( fmtCtx, NULL ) >= 0 )
	{
		for ( unsigned i = 0 ; i < fmtCtx->nb_streams ; ++i )
			if ( fmtCtx->streams[ i ]->codec->codec_type == AVMEDIA_TYPE_AUDIO )
			{
				aStream = fmtCtx->streams[ i ];
				break;
			}
		if ( aStream && !avcodec_open2( aStream->codec, avcodec_find_decoder( aStream->codec->codec_id ), NULL ) )
		{
			aCodec = aStream->codec;
			channels = aCodec->channels;
			swr = swr_alloc_set_opts( NULL, av_get_default_channel_layout( outputsCount() ), AV_SAMPLE_FMT_FLT, srate, av_get_default_channel_layout( channels ), aCodec->sample_fmt, aCodec->sample_rate, 0, NULL );
			av_opt_set_int( swr, "linear_interp", true, 0 );
			if ( !swr_init( swr ) )
			{
				dynamic_cast< FFMpegInUI * >( settings->getAdditionalSettings() )->setMaxTime( ( duration = fmtCtx->duration / AV_TIME_BASE ) );
				outBufferSize = outBufferPos = lastTime = 0;
				mutex.unlock();
				return true;
			}
		}
	}
	return false;
}
void FFMpegIn::ffmpegStop()
{
	aStream = NULL;
	aCodec = NULL;
	swr_free( &swr );
	avformat_close_input( &fmtCtx );
	outBuffer.clear();
}

void FFMpegIn::seek( int t )
{
	mutex.lock();
	if ( !av_seek_frame( fmtCtx, -1, ( int64_t )t * AV_TIME_BASE, t < lastTime ? AVSEEK_FLAG_BACKWARD : 0 ) )
	{
		avcodec_flush_buffers( aCodec );
		lastTime = t;
	}
	mutex.unlock();
}

#include <QPushButton>
#include <QToolButton>
#include <QFileDialog>
#include <QCheckBox>
#include <QLineEdit>
#include <QSlider>
#include <QLayout>

FFMpegInUI::FFMpegInUI( FFMpegIn &block ) :
	AdditionalSettings( block ),
	block( block )
{
	seekS = new QSlider( Qt::Horizontal );
	seekS->setDisabled( true );

	fileE = new QLineEdit;
	fileE->setPlaceholderText( "Ścieżka do pliku" );

	QToolButton *browseB = new QToolButton;
	browseB->setToolTip( "Przeglądaj" );
	browseB->setText( "..." );

	loopCB = new QCheckBox( "Zapętl" );

	QPushButton *applyB = new QPushButton( "&Zastosuj" );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( fileE, 0, 0, 1, 1 );
	layout->addWidget( browseB, 0, 1, 1, 1 );
	layout->addWidget( loopCB, 1, 0, 1, 2 );
	layout->addWidget( applyB, 2, 0, 1, 2 );
	layout->addWidget( seekS, 3, 0, 1, 2 );
	layout->setMargin( 3 );

	connect( browseB, SIGNAL( clicked() ), this, SLOT( browseFile() ) );
	connect( applyB, SIGNAL( clicked() ), this, SLOT( apply() ) );
	connect( seekS, SIGNAL( sliderMoved( int ) ), this, SLOT( seek( int ) ) );
	connect( this, SIGNAL( setTime( int ) ), this, SLOT( setTimeSlot( int ) ) );
}

void FFMpegInUI::prepare()
{
	fileE->setText( block.file );
	loopCB->setChecked( block.loop );
}
void FFMpegInUI::setRunMode( bool b )
{
	if ( !b )
		setMaxTime( 0 );
}

void FFMpegInUI::setMaxTime( int maxT )
{
	seekS->setMaximum( maxT > 0 ? maxT : 0 );
	seekS->setEnabled( maxT > 0 );
}

void FFMpegInUI::setTimeSlot( int t )
{
	if ( !seekS->isSliderDown() )
		seekS->setValue( t );
}
void FFMpegInUI::seek( int t )
{
	if ( block.srate )
		block.seek( t );
}
void FFMpegInUI::browseFile()
{
	QString newFile = QFileDialog::getOpenFileName( this, "Wybierz plik dźwiękowy", fileE->text() );
	if ( !newFile.isEmpty() )
		fileE->setText( newFile );
}
void FFMpegInUI::apply()
{
	const QByteArray newFile = fileE->text().toUtf8();
	if ( block.file != newFile )
	{
		block.mutex.lock();
		block.file = newFile;
		if ( block.srate )
		{
			block.ffmpegStop();
			if ( !block.ffmpegStart() )
			{
				block.ffmpegStop();
				setMaxTime( 0 );
			}
			seekS->setValue( 0 );
		}
		block.mutex.unlock();
	}
	block.loop = loopCB->isChecked();
}
