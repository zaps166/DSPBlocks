#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#ifndef Q_OS_WIN
	#include <time.h>
#else
	#include <windows.h>
#endif

#include <QtGlobal>

namespace Functions
{
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
}

#endif // FUNCTIONS_HPP
