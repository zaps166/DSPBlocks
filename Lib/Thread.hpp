#ifndef THREAD_HPP
#define THREAD_HPP

#include <QThread>
#include <QVector>

class Block;

class Thread : public QThread
{
	Q_OBJECT
public:
#ifdef Q_OS_LINUX
	inline qint32 getRealSampleRate() const
	{
		return realSampleRate;
	}
	inline bool isRealTimeNow()
	{
		return isRunning() && rt;
	}
#endif

	void start( const QVector< Block * > &sources, quint64 simSamples, bool isBlocking );
	bool stop();
signals:
	void errorMessage( const QString &msg );
private:
	void run();

	QVector< Block * > sources;
	quint64 simSamples;
	bool isBlocking;
	volatile bool br;

#ifdef Q_OS_LINUX
	qint32 realSampleRate;
	bool rt;
#endif
};

#endif // THREAD_HPP
