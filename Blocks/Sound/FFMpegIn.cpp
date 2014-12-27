#include "FFMpegIn.hpp"
#include "Global.hpp"
#include "Array.hpp"

#include <QDebug>

extern "C"
{
	#include <libavformat/avformat.h>
	#include <libswresample/swresample.h>
	#include <libavutil/opt.h>
}

#ifdef Q_OS_LINUX
	#include <pthread.h>
	#include <sched.h>
#endif

FFMpegDec::FFMpegDec( FFMpegIn &block ) :
	block( block ),
	fmtCtx( NULL ), aStream( NULL ), aCodec( NULL ), swr( NULL ),
	frame( ( AVFrame * )av_mallocz( sizeof *frame ) ),
	canGetBuffer( false )
{}
FFMpegDec::~FFMpegDec()
{
	av_free( frame );
}

bool FFMpegDec::start()
{
	lastTime = get_buffer_idx = buffers_available = 0;
	canGetBuffer = stopped = true;
	seekTo = -1;
	br = false;
	if ( avformat_open_input( &fmtCtx, block.file, NULL, NULL ) == 0 && avformat_find_stream_info( fmtCtx, NULL ) >= 0 )
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
			swr = swr_alloc_set_opts( NULL, av_get_default_channel_layout( block.outputsCount() ), AV_SAMPLE_FMT_FLT, Global::getSampleRate(), av_get_default_channel_layout( channels ), aCodec->sample_fmt, aCodec->sample_rate, 0, NULL );
			av_opt_set_int( swr, "linear_interp", true, 0 );
			if ( !swr_init( swr ) )
			{
				dynamic_cast< FFMpegInUI * >( block.settings->getAdditionalSettings() )->setMaxTime( ( duration = fmtCtx->duration / AV_TIME_BASE ) );
				QThread::start();
				return true;
			}
		}
	}
	return false;
}
void FFMpegDec::stop()
{
	block.mutex.lock();
	canGetBuffer = false;
	block.outBuffer = NULL;
	block.mutex.unlock();
	if ( isRunning() )
	{
		buffer_mutex.lock();
		br = true;
		if ( fmtCtx && fmtCtx->pb )
			fmtCtx->pb->eof_reached = true;
		buffer_cond.wakeOne();
		buffer_mutex.unlock();
		wait();
	}
	aStream = NULL;
	aCodec = NULL;
	swr_free( &swr );
	avformat_close_input( &fmtCtx );
	for ( int i = 0 ; i < NUM_BUFFERS ; ++i )
		outBuffer[ i ].clear();
}

const float *FFMpegDec::getBuffer()
{
	buffer_mutex.lock();
	if ( !canGetBuffer || ( !buffers_available && block.nonblocking ) )
	{
		buffer_mutex.unlock();
		return NULL;
	}
	if ( !buffers_available && !stopped )
		get_buffer_cond.wait( &buffer_mutex );
	if ( buffers_available )
	{
		buffer_mutex.unlock();
		block.outBufferSize = outBufferSamples[ get_buffer_idx ];
		const float *buffer = outBuffer[ get_buffer_idx ].data();
		block.outBufferPos = 0;
		if ( ++get_buffer_idx == NUM_BUFFERS )
			get_buffer_idx = 0;
		return buffer;
	}
	buffer_mutex.unlock();
	return NULL;
}
void FFMpegDec::releaseBuffer()
{
	buffer_mutex.lock();
	--buffers_available;
	buffer_cond.wakeOne();
	buffer_mutex.unlock();
}

void FFMpegDec::run()
{
	int buffer_idx = 0;
	if ( block.highPriority )
	{
#ifdef Q_OS_LINUX
		sched_param param = { 1 };
		if ( pthread_setschedparam( pthread_self(), SCHED_RR, &param ) )
			perror( "Nie można ustawić SCHED_RR dla wątku FFMpeg" );
		else
#endif
			setPriority( HighestPriority );
	}
	while ( !br )
	{
		AVPacket pkt;
		pkt.data = NULL;
		int e = 0;
		while ( !br && ( e = av_read_frame( fmtCtx, &pkt ) ) >= 0 )
		{
			int outBufferSize = 0;
			stopped = false;
			if ( pkt.stream_index == aStream->index )
			{
				AVPacket pkt_tmp = pkt;
				int ok = 0, bread;
				while ( ( bread = avcodec_decode_audio4( aCodec, frame, &ok, &pkt_tmp ) ) >= 0 && ok )
				{
					if ( aCodec->channels == channels )
					{
						const int out_size = ceil( frame->nb_samples * ( float )Global::getSampleRate() / ( float )aCodec->sample_rate );
						int buffer_size = outBufferSize + out_size * block.outputsCount();

						if ( outBuffer[ buffer_idx ].size() < buffer_size )
							outBuffer[ buffer_idx ].resize( buffer_size );
						quint8 *out[] = { ( quint8 * )( outBuffer[ buffer_idx ].data() + outBufferSize ) };
						if ( ( buffer_size = swr_convert( swr, out, out_size, ( const quint8 ** )frame->extended_data, frame->nb_samples ) ) > 0 )
							outBufferSize += buffer_size * block.outputsCount();
					}
					if ( duration > 0 )
					{
						double currentTime = av_frame_get_best_effort_timestamp( frame ) * av_q2d( aStream->time_base );
						if ( currentTime - lastTime >= 1.0 )
						{
							lastTime = ( int )currentTime;
							emit dynamic_cast< FFMpegInUI * >( block.settings->getAdditionalSettings() )->setTime( lastTime );
						}
					}
					pkt_tmp.data += bread;
					pkt_tmp.size -= bread;
				}
			}
			av_free_packet( &pkt );
			if ( outBufferSize )
			{
				outBufferSamples[ buffer_idx ] = outBufferSize;
				if ( ++buffer_idx >= NUM_BUFFERS )
					buffer_idx = 0;
				buffer_mutex.lock();
				if ( !br )
				{
					++buffers_available;
					get_buffer_cond.wakeOne();
					if ( buffers_available == NUM_BUFFERS )
						buffer_cond.wait( &buffer_mutex );
				}
				buffer_mutex.unlock();
			}
			if ( seekTo > -1 )
				break;
		}
		if ( seekTo > -1 )
		{
			if ( !br && !av_seek_frame( fmtCtx, -1, ( int64_t )seekTo * AV_TIME_BASE, seekTo < lastTime ? AVSEEK_FLAG_BACKWARD : 0 ) )
			{
				avcodec_flush_buffers( aCodec );
				lastTime = seekTo;
			}
			seekTo = -1;
		}
		else if ( !br && e == AVERROR_EOF )
		{
			if ( block.loop && !av_seek_frame( fmtCtx, -1, 0, AVSEEK_FLAG_BACKWARD ) )
			{
				avcodec_flush_buffers( aCodec );
				lastTime = 0;
			}
			else
			{
				if ( !stopped )
				{
					buffer_mutex.lock();
					stopped = true;
					get_buffer_cond.wakeOne();
					buffer_mutex.unlock();
				}
				msleep( 100 );
			}
		}
	}
}

/**/

FFMpegIn::FFMpegIn() :
	Block( "FFMpeg input", "Odtwarza pliki dźwiękowe", 0, 1, SOURCE ),
	ffdec( *this ),
	loop( false ), highPriority( false ), nonblocking( Global::isRealTime() )
{}

bool FFMpegIn::start()
{
	outBuffer = NULL;
	settings->setRunMode( true );
	if ( !ffdec.start() )
	{
		if ( file.isEmpty() )
			err = "Podaj ścieżkę do pliku!";
		else if ( !QFile::exists( file ) )
			err = "Podany plik nie istnieje!";
		else
			err = "Sprawdź poprawność pliku!";
		return false;
	}
	return true;
}
void FFMpegIn::exec( Array< Sample > &samples )
{
	mutex.lock();
	if ( !outBuffer )
		outBuffer = ffdec.getBuffer();
	if ( outBuffer )
	{
		for ( int i = 0 ; i < outputsCount() ; ++i )
			samples += ( Sample ){ getTarget( i ), outBuffer[ outBufferPos++ ] };
		if ( !( outBufferSize -= outputsCount() ) )
		{
			ffdec.releaseBuffer();
			outBuffer = NULL;
		}
	}
	else for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), 0.0f };
	mutex.unlock();
}
void FFMpegIn::stop()
{
	settings->setRunMode( false );
	ffdec.stop();
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
	ds << file << loop << highPriority << nonblocking;
}
void FFMpegIn::deSerialize( QDataStream &ds )
{
	ds >> file >> loop >> highPriority >> nonblocking;
}

#include <QPushButton>
#include <QToolButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QSlider>
#include <QLayout>

FFMpegInUI::FFMpegInUI( FFMpegIn &block ) :
	AdditionalSettings( block ),
	block( block ),
	isRunning( false )
{
	seekS = new QSlider( Qt::Horizontal );
	seekS->setDisabled( true );

	fileE = new QLineEdit;
	fileE->setPlaceholderText( "Ścieżka do pliku" );

	QToolButton *browseB = new QToolButton;
	browseB->setToolTip( "Przeglądaj" );
	browseB->setText( "..." );

	loopCB = new QCheckBox( "Zapętl odtwarzanie" );
	priorityCB = new QCheckBox( "Wysoki priorytet" );
	nonblockingCB = new QCheckBox( "Tryb nieblokujący" );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( fileE, 0, 0, 1, 1 );
	layout->addWidget( browseB, 0, 1, 1, 1 );
	layout->addWidget( seekS, 1, 0, 1, 2 );
	layout->addWidget( loopCB, 2, 0, 1, 2 );
	layout->addWidget( priorityCB, 3, 0, 1, 2 );
	layout->addWidget( nonblockingCB, 4, 0, 1, 2 );
	layout->setMargin( 3 );

	connect( fileE, SIGNAL( editingFinished() ), this, SLOT( setNewFile() ) );
	connect( browseB, SIGNAL( clicked() ), this, SLOT( browseFile() ) );
	connect( seekS, SIGNAL( sliderMoved( int ) ), this, SLOT( seek( int ) ) );
	connect( loopCB, SIGNAL( clicked( bool ) ), this, SLOT( setLoop( bool ) ) );
	connect( priorityCB, SIGNAL( clicked( bool ) ), this, SLOT( setHighPriority( bool ) ) );
	connect( nonblockingCB, SIGNAL( clicked( bool ) ), this, SLOT( setNonblocking( bool ) ) );
	connect( this, SIGNAL( setTime( int ) ), this, SLOT( setTimeSlot( int ) ) );
}

void FFMpegInUI::prepare()
{
	fileE->setText( block.file );
	loopCB->setChecked( block.loop );
	priorityCB->setChecked( block.highPriority );
	nonblockingCB->setChecked( block.nonblocking );
}
void FFMpegInUI::setRunMode( bool b )
{
	if ( !b )
		setMaxTime( 0 );
	priorityCB->setDisabled( b );
	isRunning = b;
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
	block.ffdec.seek( t );
}
void FFMpegInUI::browseFile()
{
	QString newFile = QFileDialog::getOpenFileName( this, "Wybierz plik dźwiękowy", fileE->text(), QString(), NULL, Global::getNativeFileDialogFlag() );
	if ( !newFile.isEmpty() )
	{
		fileE->setText( newFile );
		setNewFile();
	}
}
void FFMpegInUI::setLoop( bool loop )
{
	block.loop = loop;
}
void FFMpegInUI::setHighPriority( bool highPriority )
{
	block.highPriority = highPriority;
}
void FFMpegInUI::setNonblocking( bool nonblocking )
{
	block.nonblocking = nonblocking;
}
void FFMpegInUI::setNewFile()
{
	const QByteArray newFile = fileE->text().toUtf8();
	if ( block.file != newFile )
	{
		block.file = newFile;
		if ( isRunning )
		{
			block.ffdec.stop();
			if ( !block.ffdec.start() )
			{
				block.ffdec.stop();
				setMaxTime( 0 );
			}
			seekS->setValue( 0 );
		}
	}
}
