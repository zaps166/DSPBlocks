#include "DrawHelper.hpp"
#include "Functions.hpp"
#include "Block.hpp"

void DrawThr::start()
{
	br = false;
	QThread::start();
}
void DrawThr::stop()
{
	drawHelper.drawMutex.lock();
	br = true;
	drawHelper.drawCond.wakeOne();
	drawHelper.drawMutex.unlock();
	wait();
}

void DrawThr::run()
{
	qint64 t1 = Functions::gettime();
	qint32 period = Block::getPeriod() * 1000000;
	drawHelper.drawMutex.lock();
	while ( !br )
	{
		if ( !br && !drawHelper.bufferReady )
			drawHelper.drawCond.wait( &drawHelper.drawMutex );
		if ( !br )
		{
			qint64 t2 = Functions::gettime();
			if ( ( t2 - t1 ) / 1000 >= period )
			{
				drawHelper.draw();
				t1 = t2;
			}
			drawHelper.bufferReady = false;
		}
	}
	drawHelper.drawMutex.unlock();
}
