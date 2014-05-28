#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#ifndef Q_OS_WIN
	#include <time.h>
#else
	#include <windows.h>
#endif

namespace Functions
{
	static inline double gettime()
	{
#ifndef Q_OS_WIN
		timespec now;
		clock_gettime( CLOCK_MONOTONIC, &now );
		return now.tv_sec + ( now.tv_nsec / 1000000000.0 );
#else
		LARGE_INTEGER Frequency, Counter;
		QueryPerformanceFrequency( &Frequency );
		QueryPerformanceCounter( &Counter );
		return ( double )Counter.QuadPart / ( double )Frequency.QuadPart;
#endif
	}
}

#endif // FUNCTIONS_HPP
