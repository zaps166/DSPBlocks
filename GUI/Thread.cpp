#include "Thread.hpp"
#include "Global.hpp"
#include "Block.hpp"
#include "Array.hpp"

#define SECOND (1000000000LL)

#ifdef Q_OS_WIN
	#include <mmsystem.h>
#endif
#ifdef Q_OS_LINUX
	#include <sys/time.h>
//	#include <sys/mman.h>
	#include <pthread.h>
	#include <sched.h>

	#ifdef USE_RTAI
		#include <rtai_lxrt.h>
	#endif
#endif

#include <QDebug>

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
void Thread::stop()
{
	if ( isRunning() )
	{
		br = true;
		wait();
#ifdef Q_OS_WIN
		timeEndPeriod( 1 );
#endif
	}
}

void Thread::run()
{
	Array< Block::Sample > samples;
	Array< Block * > blocks/*, crossBlocksStart, crossBlocksStart2*/;

	quint32 counter = 0, counter_ov = 0, period;
	qint64 t1 = 0;
#ifdef Q_OS_LINUX
	qint64 realSrateT = 0;
	timespec rt_time;
	#ifdef USE_RTAI
		RT_TASK *rtai_task;
		RTIME rtai_period;
	#endif
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
#ifdef USE_RTAI
		if ( rt_mode != RTAI )
		{
#endif
			sched_param param = { Global::getPriority() };
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
	//		if ( mlockall( MCL_CURRENT | MCL_FUTURE ) )
	//			perror( "mlockall" );
			if ( !errStr.isEmpty() )
			{
				errStr += "\nSprawdź uprawnienia!";
				emit errorMessage( errStr );
			}
#ifdef USE_RTAI
		}
#endif
		period = SECOND / Global::getSampleRate();
		counter_ov = Global::getSampleRate();
		realSrateT = Global::gettime();
		switch ( Global::getRtMode() )
		{
			case Global::NANOSLEEP:
				t1 = realSrateT;
				break;
#ifdef USE_RTAI
			case RTAI:
			{
				int cpu_allowed = Thread::getCPU() ? ( 1 << ( Thread::getCPU() - 1 ) ) : 0;
				rtai_task = rt_task_init_schmod( NULL, Thread::getPriority(), 0, 1, Thread::getSched(), cpu_allowed );
				if ( rt_is_hard_timer_running() )
					stop_rt_timer();
				rt_set_oneshot_mode();
				rtai_period = nano2count( period );
				start_rt_timer( 0 );
				rt_task_make_periodic( rtai_task, rt_get_time() + rtai_period, rtai_period );
				rt_make_hard_real_time();
			} break;
#endif
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
			{
//				printf( "%s	", blocks[ i ]->getName().toUtf8().data() );
//				int sc = samples.count();
				blocks[ i ]->exec( samples );
//				for ( int sci = sc ; sci < samples.count() ; ++sci )
				{
//					printf( "%.0f ", samples[ sci ].sample );
				}
//				putchar( 10 );
//				fflush( stdout );
			}
			blocks.clear();
			for ( int i = 0 ; i < samples.count() ; ++i )
			{
				Block *block = samples[ i ].target.first;
				if ( block )
				{
//					bool e = false;
//					for ( int j = 0 ; j < crossBlocksStart2.count() ; ++j )
//						if ( block == crossBlocksStart2[ j ] )
//						{
//							e = true;
//							break;
//						}
//					if ( e )
//						continue;

					block->setSample( samples[ i ].target.second, samples[ i ].sample );
					if ( !block->allConnectedInputsHasSample() )
					{
//						crossBlocksStart += block;
					}
					else
					{
						blocks += block;
//						for ( int i = 0 ; i < crossBlocksStart.count() ; ++i )
//							if ( crossBlocksStart[ i ] == block )
//								crossBlocksStart[ i ] = NULL;
					}
				}
			}
			samples.clear();
		}

//		crossBlocksStart2.clear();
//		for ( int i = 0 ; i < crossBlocksStart.count() ; ++i )
//		{
//			Block *block = crossBlocksStart[ i ];
//			if ( block )
//			{
//				blocks += block;
//				crossBlocksStart2 += block;
//				block->setInputsDone();
//				qDebug() << block->getName();
//			}
//		}
//		crossBlocksStart.clear();
//		if ( blocks.notEmpty() )
//			continue;

//		usleep( 0 );

#ifdef Q_OS_LINUX
		if ( rt )
		{
			switch ( Global::getRtMode() )
			{
				case Global::NANOSLEEP:
				{
					qint64 t2 = Global::gettime();
					long sleep_time = period - t2 + t1;
					if ( sleep_time > 0 )
					{
						timespec ts = { 0, sleep_time };
						nanosleep( &ts, NULL );
					}
					t1 = Global::gettime();
					t1 -= t1 - t2 - sleep_time;
				} break;
#ifdef USE_RTAI
				case RTAI:
					rt_task_wait_period();
					break;
#endif
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
//		putchar( 10 );
//		fflush( stdout );
	}

#ifdef USE_RTAI
	if ( rt_mode == RTAI )
	{
		rt_make_soft_real_time();
		rt_task_delete( rtai_task );
		stop_rt_timer();
	}
#endif

	sources.clear();
}
