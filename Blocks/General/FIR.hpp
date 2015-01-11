#ifndef FIR_HPP
#define FIR_HPP

#include "Block.hpp"
#include "RingBuffer.hpp"

#include <QMutex>

class FIR : public Block
{
	friend class FIR_UI;
public:
	FIR();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void setInputBuffer();

	QScopedArrayPointer< RingBuffer< float > >inputBuffer;
	QScopedArrayPointer< float > inputSamples;
	QVector< float > fir_coeff;
	QMutex mutex;
};

#include "Settings.hpp"

class QPlainTextEdit;
class QPushButton;
class QLabel;

class FIR_UI : public AdditionalSettings
{
	Q_OBJECT
public:
	FIR_UI( FIR &block );

	void prepare();
	bool canClose();
private slots:
	void toggleLiveUpdate( bool l );
	void setFilterLenInfo( int l );
	void setFilter();
private:
	QPlainTextEdit *coeffE;
	QPushButton *applyB;
	QLabel *filterLenL;

	bool canUpdateFilter;

	FIR &block;
};

#endif // FIR_HPP
