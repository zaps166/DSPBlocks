#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <QFileDialog>

#include <math.h>

#ifdef Q_OS_WIN
	#include <windows.h>
#else
	#ifdef Q_OS_MAC
		#include <mach/mach_time.h>
	#else
		#include <time.h>
	#endif
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
#if defined Q_OS_WIN
		LARGE_INTEGER Frequency, Counter;
		QueryPerformanceFrequency( &Frequency );
		QueryPerformanceCounter( &Counter );
		return 1000000000.0 * ( ( double )Counter.QuadPart / ( double )Frequency.QuadPart ); //64bit integer can overflow
#elif defined Q_OS_MAC
		mach_timebase_info_data_t mach_base_info;
		mach_timebase_info( &mach_base_info );
		return ( mach_absolute_time() * mach_base_info.numer ) / mach_base_info.denom;
#else
		timespec now;
		clock_gettime(
		#ifdef CLOCK_MONOTONIC_RAW
			CLOCK_MONOTONIC_RAW,
		#else
			CLOCK_MONOTONIC,
		#endif
			&now
		);
		return now.tv_sec * 1000000000LL + now.tv_nsec;
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
#else
	static inline bool isRealTime()
	{
		return false;
	}
#endif
private:
	static bool nativeFileDialogFlag;
	static int sampleRate, refTime;

#ifdef Q_OS_LINUX
	static bool realTime;
	static RT_MODE rt_mode;
	static int cpu, sched, priority;
#endif
};

#endif // GLOBAL_HPP
