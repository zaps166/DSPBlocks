#include "Thread.hpp"
#include "Global.hpp"
#include "Block.hpp"
#include "Array.hpp"

#define SECOND (1000000000LL)

#ifdef Q_OS_WIN
	#include <mmsystem.h>
#endif
#ifdef Q_OS_LINUX
	#ifdef USE_MLOCKALL
		#include <sys/mman.h>
	#endif
	#include <sys/time.h>
	#include <pthread.h>
	#include <sched.h>
#endif

#include <QApplication>

void Thread::start( const QVector< Block * > &sources, quint64 simSamples, bool isBlocking )
{
	this->sources = sources;
	this->simSamples = simSamples;
	this->isBlocking = isBlocking;
	br = false;

#ifdef Q_OS_WIN
	timeBeginPeriod( 1 );
#endif

#ifdef Q_OS_LINUX
	realSampleRate = -1;
	rt = false;
#endif

	QThread::start();
}
bool Thread::stop()
{
	bool ok = true;
	if ( isRunning() )
	{
		br = true;
		qApp->setOverrideCursor( Qt::WaitCursor );
		ok = wait( 2500 );
		qApp->restoreOverrideCursor();
#ifdef Q_OS_WIN
		if ( ok )
			timeEndPeriod( 1 );
#endif
	}
	return ok;
}

void Thread::run()
{
	Array< Block::Sample > samples;
	Array< Block * > blocks;

	quint32 counter = 0, counter_ov = 0, period;
	qint64 t1 = 0;
#ifdef Q_OS_LINUX
	qint64 realSrateT = 0;
	timespec rt_time;
	rt = isBlocking ? false : Global::isRealTime();

	if ( !rt )
	{
#endif
		period = Global::getPeriod( 0.0 ) * SECOND;
		isBlocking |= period == 0;
		if ( !isBlocking )
		{
			counter_ov = Global::getBufferSize();
			t1 = Global::gettime();
		}
#ifdef Q_OS_LINUX
	}
	else
	{
		QString errStr;
		const sched_param param = { Global::getPriority() };
		if ( pthread_setschedparam( pthread_self(), Global::getSched(), &param ) )
			errStr = "Nie można ustawić schedulera.";
		if ( Global::getCPU() )
		{
			cpu_set_t set;
			CPU_ZERO( &set );
			CPU_SET( Global::getCPU() - 1, &set );
			if ( pthread_setaffinity_np( pthread_self(), sizeof set, &set ) )
			{
				if ( !errStr.isEmpty() )
					errStr += '\n';
				errStr += "Nie można ustawić CPU affinity.";
			}
		}
#ifdef USE_MLOCKALL
		if ( mlockall( MCL_CURRENT | MCL_FUTURE ) )
			perror( "mlockall failed" );
#endif
		if ( !errStr.isEmpty() )
		{
			errStr += "\nSprawdź uprawnienia!";
			emit errorMessage( errStr );
		}
		period = SECOND / Global::getSampleRate();
		counter_ov = Global::getSampleRate();
		realSrateT = Global::gettime();
		switch ( Global::getRtMode() )
		{
			case Global::NANOSLEEP:
				t1 = realSrateT;
				break;
			case Global::CLOCK_NANOSLEEP:
			default:
				clock_gettime( CLOCK_MONOTONIC, &rt_time );
				break;
		}
	}
#endif

	while ( !br )
	{
		if ( !blocks.notEmpty() )
			blocks = sources;
		while ( blocks.notEmpty() )
		{
			for ( int i = 0 ; i < blocks.count() ; ++i )
				blocks[ i ]->exec( samples );
			blocks.clear();
			for ( int i = 0 ; i < samples.count() ; ++i )
			{
				Block *block = samples[ i ].target.first;
				if ( block )
				{
					block->setSample( samples[ i ].target.second, samples[ i ].sample );
					if ( block->allConnectedInputsHasSample() )
						blocks += block;
				}
			}
			samples.clear();
		}
#ifdef Q_OS_LINUX
		if ( rt )
		{
			switch ( Global::getRtMode() )
			{
				case Global::NANOSLEEP:
				{
					qint64 t2 = Global::gettime();
					qint64 sleep_time = period - t2 + t1;
					if ( sleep_time > 0 )
					{
						timespec ts = { 0, ( long )sleep_time };
						nanosleep( &ts, NULL );
					}
					t1 = Global::gettime();
					t1 -= t1 - t2 - sleep_time;
				} break;
				case Global::CLOCK_NANOSLEEP:
				default:
					clock_nanosleep( CLOCK_MONOTONIC, TIMER_ABSTIME, &rt_time, NULL );
					rt_time.tv_nsec += period;
					while ( rt_time.tv_nsec >= SECOND )
					{
						rt_time.tv_nsec -= SECOND;
						++rt_time.tv_sec;
					}
					break;
			}
			if ( ++counter == counter_ov )
			{
				qint64 realSrateT2 = Global::gettime();
				realSampleRate = counter_ov * ( SECOND * 10LL ) / ( realSrateT2 - realSrateT );
				realSrateT = realSrateT2;
				counter = 0;
			}
		}
		else
#endif
		if ( !isBlocking && ++counter == counter_ov )
		{
			qint64 t2 = Global::gettime();
			qint64 sleep_time = period - t2 + t1;
			if ( sleep_time >= 1000 )
				usleep( sleep_time / 1000 );
			t1 = Global::gettime();
			t1 -= t1 - t2 - sleep_time;
			counter = 0;
		}
		if ( simSamples && !--simSamples )
			break;
	}

	sources.clear();

#if defined Q_OS_LINUX && defined USE_MLOCKALL
	if ( rt && munlockall() )
		 perror( "munlockall failed" );
#endif
}
