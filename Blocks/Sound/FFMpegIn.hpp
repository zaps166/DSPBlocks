#ifndef FFMPEGIN_HPP
#define FFMPEGIN_HPP

#include "Block.hpp"

struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
struct SwrContext;
struct AVFrame;

class FFMpegIn;

#include <QWaitCondition>
#include <QThread>
#include <QMutex>

#define NUM_BUFFERS 3

class FFMpegDec : public QThread
{
public:
	FFMpegDec( FFMpegIn &block );
	~FFMpegDec();

	bool start();
	void stop();

	inline void seek( int t )
	{
		seekTo = t;
	}

	const float *getBuffer();
	void releaseBuffer();
private:
	void run();

	FFMpegIn &block;
	volatile bool br;

	AVFormatContext *fmtCtx;
	AVStream *aStream;
	AVCodecContext *aCodec;
	SwrContext *swr;
	AVFrame *frame;

	QVector< float > outBuffer[ NUM_BUFFERS ];
	int outBufferSamples[ NUM_BUFFERS ];
	int channels, lastTime, duration, seekTo, get_buffer_idx, buffers_available;
	bool canGetBuffer, stopped;

	QWaitCondition buffer_cond, get_buffer_cond;
	QMutex buffer_mutex;
};

class FFMpegIn : public Block
{
	friend class FFMpegInUI;
	friend class FFMpegDec;
public:
	FFMpegIn();

	bool start();
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	FFMpegDec ffdec;

	int outBufferSize, outBufferPos;
	const float *outBuffer;

	QByteArray file;
	QMutex mutex;

	bool loop, highPriority, nonblocking;
};

#include "Settings.hpp"

class QCheckBox;
class QLineEdit;
class QSlider;

class FFMpegInUI : public AdditionalSettings
{
	Q_OBJECT
public:
	FFMpegInUI( FFMpegIn &block );

	void prepare();
	void setRunMode( bool b );

	void setMaxTime( int maxT );
	Q_SIGNAL void setTime( int t );
private slots:
	void setTimeSlot( int t );
	void seek( int t );
	void browseFile();
	void setLoop( bool loop );
	void setHighPriority( bool highPriority );
	void setNonblocking( bool nonblocking );
	void setNewFile();
private:
	FFMpegIn &block;
	bool isRunning;

	QLineEdit *fileE;
	QCheckBox *loopCB, *priorityCB, *nonblockingCB;
	QSlider *seekS;
};

#endif // FFMPEGIN_HPP
