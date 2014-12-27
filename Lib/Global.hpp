#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <QFileDialog>

#include <math.h>

#ifndef Q_OS_WIN
	#include <time.h>
#else
	#include <windows.h>
#endif

#define DEFAULT_SAMPLERATE 48000
#define DEFAULT_REFTIME 60

#ifdef Q_OS_LINUX
	#define DEFAULT_PRIORITY 99
#endif

class Block;

class Global
{
public:
	static inline qint64 gettime()
	{
#ifndef Q_OS_WIN
		timespec now;
		clock_gettime( CLOCK_MONOTONIC, &now );
		return now.tv_sec * 1000000000LL + now.tv_nsec;
#else
		LARGE_INTEGER Frequency, Counter;
		QueryPerformanceFrequency( &Frequency );
		QueryPerformanceCounter( &Counter );
		return Counter.QuadPart * 1000000000LL / Frequency.QuadPart;
#endif
	}

	static inline void setNativeFileDialog( bool n )
	{
		nativeFileDialogFlag = n;
	}
	static inline QFileDialog::Option getNativeFileDialogFlag()
	{
		return nativeFileDialogFlag ? ( QFileDialog::Option )0 : QFileDialog::DontUseNativeDialog;
	}
	static inline bool isNativeFileDialog()
	{
		return nativeFileDialogFlag;
	}

	static inline void setSampleRateAndRefTime( int srate, int refT )
	{
		sampleRate = qMax( srate, 1 );
		refTime = qMin( refT, sampleRate );
	}
	static inline void resetSampleRateAndRefTime()
	{
		sampleRate = DEFAULT_SAMPLERATE;
		refTime = DEFAULT_REFTIME;
	}
	static inline int getSampleRate()
	{
		return sampleRate;
	}
	static inline int getRefTime()
	{
		return refTime;
	}
	static inline double getPeriod( double fallback = 1.0 / DEFAULT_REFTIME )
	{
		return refTime ? ( 1.0 / refTime ) : fallback;
	}
	static inline int getBufferSize( double fallback = DEFAULT_REFTIME )
	{
		return ceil( ( double )sampleRate / ( double )( refTime ? refTime : fallback ) );
	}

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
#endif
private:
	static bool nativeFileDialogFlag;
	static int sampleRate, refTime;

	static bool realTime;
	static RT_MODE rt_mode;
	static int cpu, sched, priority;
};

#endif // GLOBAL_HPP
