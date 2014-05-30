#ifndef FFMPEGIN_HPP
#define FFMPEGIN_HPP

#include "Block.hpp"

struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
struct SwrContext;
struct AVFrame;

#include <QMutex>

class FFMpegIn : public Block
{
	friend class FFMpegInUI;
public:
	FFMpegIn();
	~FFMpegIn();

	bool start();
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	bool ffmpegStart();
	void ffmpegStop();

	void seek( int t );

	QByteArray file;
	QMutex mutex;

	QVector< float > outBuffer;
	int outBufferPos, outBufferSize;

	AVFormatContext *fmtCtx;
	AVStream *aStream;
	AVCodecContext *aCodec;
	SwrContext *swr;
	AVFrame *frame;
	int srate, channels;
	int duration, lastTime;
	bool loop;
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
	void apply();
private:
	FFMpegIn &block;

	QLineEdit *fileE;
	QCheckBox *loopCB;
	QSlider *seekS;
};

#endif // FFMPEGIN_HPP
