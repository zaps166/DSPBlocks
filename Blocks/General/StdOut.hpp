#ifndef STDOUT_HPP
#define STDOUT_HPP

#include "Block.hpp"

class StdOut : public Block
{
	friend class StdOutUI;
public:
	StdOut();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	QVector< float > buffer, lastBuffer;
	bool boolean;
};

#include "Settings.hpp"

class QCheckBox;

class StdOutUI : public AdditionalSettings
{
	Q_OBJECT
public:
	StdOutUI( StdOut &block );

	void prepare();
private slots:
	void setValue( bool b );
private:
	StdOut &block;

	QCheckBox *booleanB;
};

#endif // STDOUT_HPP
