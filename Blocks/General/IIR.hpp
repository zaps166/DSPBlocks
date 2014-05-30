#ifndef IIR_HPP
#define IIR_HPP

#include "Block.hpp"
#include "RingBuffer.hpp"

#include <QMutex>

class IIR : public Block
{
	friend class IIR_UI;
public:
	IIR();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void setInputBuffer();

	RingBuffer< float > *inputBuffer;
	RingBuffer< double > *outputBuffer;
	QVector< double > Acoeff, Bcoeff;
	QVector< float > inputSamples;
	QMutex mutex;
};

#include "Settings.hpp"

class QPlainTextEdit;
class QPushButton;
class QLabel;

class IIR_UI : public AdditionalSettings
{
	Q_OBJECT
public:
	IIR_UI( IIR &block );

	void prepare();
	bool canClose();
private slots:
	void toggleLiveUpdate( bool l );
	void setFilterLenInfo();
	void setFilter();
private:
	QPlainTextEdit *AcoeffE, *BcoeffE;
	QPushButton *applyB;
	QLabel *filterLenL;

	bool canUpdateFilter;

	IIR &block;
};

#endif // IIR_HPP
