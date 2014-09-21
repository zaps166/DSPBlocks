#ifndef DRAWHELPER_HPP
#define DRAWHELPER_HPP

#include <QWaitCondition>
#include <QThread>
#include <QMutex>

class DrawHelper;

class DrawThr : public QThread
{
public:
	inline DrawThr( DrawHelper &drawHelper ) :
		drawHelper( drawHelper )
	{}

	void start();
	void stop();
private:
	void run();

	DrawHelper &drawHelper;

	volatile bool br;
};

class DrawHelper
{
	friend class DrawThr;
protected:
	virtual void draw() = 0;

	QWaitCondition drawCond;
	QMutex drawMutex;
	bool bufferReady;
};

#endif // DRAWHELPER_HPP
