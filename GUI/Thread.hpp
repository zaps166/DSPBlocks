#ifndef THREAD_HPP
#define THREAD_HPP

#include <QThread>
#include <QVector>

class Block;

#ifdef Q_OS_LINUX
	#define DEFAULT_PRIORITY 99
#endif

class Thread : public QThread
{
	Q_OBJECT
public:
#ifdef Q_OS_LINUX
	enum RT_MODE
	{
		CLOCK_NANOSLEEP,
		NANOSLEEP,
#ifdef USE_RTAI
		RTAI,
#endif
		RT_MODE_MAX
	};

	static inline void setRealTime( bool rt, int c, int s, int p, int _rt_mode )
	{
		realTime = rt;
		cpu = c;
		sched = s;
		priority = p;
		if ( _rt_mode < CLOCK_NANOSLEEP || _rt_mode >= RT_MODE_MAX )
			rt_mode = CLOCK_NANOSLEEP;
		else
			rt_mode = ( RT_MODE )_rt_mode;
	}
	static inline bool isRealTime()
	{
		return realTime;
	}
	static inline int getCPU()
	{
		return cpu;
	}
	static inline int getSched()
	{
		return sched;
	}
	static inline int getPriority()
	{
		return priority;
	}
	static inline RT_MODE getRtMode()
	{
		return rt_mode;
	}

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
	void stop();
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

	static bool realTime;
	static RT_MODE rt_mode;
	static int cpu, sched, priority;
#endif
};

#endif // THREAD_HPP
