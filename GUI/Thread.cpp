#include "Thread.hpp"
#include "Block.hpp"
#include "Array.hpp"
#include "Functions.hpp"

#ifdef Q_OS_WIN
	#include <mmsystem.h>
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

	quint32 counter = 0, counter_ov = 0;
	double t1 = 0.0, period = Block::getPeriod( 0.0 );
	isBlocking |= period == 0.0;
	if ( !isBlocking )
	{
		counter_ov = Block::getBufferSize();
		t1 = Functions::gettime();
	}

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

		if ( !isBlocking && ++counter == counter_ov )
		{
			double t2 = Functions::gettime();
			double sleep_time = period - t2 + t1;
			if ( sleep_time > 0.0 )
				usleep( sleep_time * 1000000 );
			t1 = Functions::gettime();
			t1 -= t1 - t2 - sleep_time;
			counter = 0;
		}
		if ( simSamples && !--simSamples )
			break;
//		putchar( 10 );
//		fflush( stdout );
	}

	sources.clear();
}
