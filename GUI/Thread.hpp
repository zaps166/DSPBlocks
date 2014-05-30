#ifndef THREAD_HPP
#define THREAD_HPP

#include <QThread>
#include <QVector>

class Block;

class Thread : public QThread
{
public:
	void start( const QVector< Block * > &sources, quint64 simSamples, bool isBlocking );
	void stop();
private:
	void run();

	QVector< Block * > sources;
	quint64 simSamples;
	bool isBlocking;
	volatile bool br;
};

#endif // THREAD_HPP
