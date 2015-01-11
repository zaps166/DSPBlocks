#ifndef RMS_HPP
#define RMS_HPP

#include "Block.hpp"

class RMS : public Block
{
	friend class RMS_UI;
public:
	RMS();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	enum { MD_RMS, MD_AVG, MD_MAX, MD_MIN };

	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void setLabel();
	void calcNumSamples();

	int ms, numSamples;
	QScopedArrayPointer< double > partial_result;
	QScopedArrayPointer< float > result;
	QAtomicInt pos;
	quint8 mode;
};

#include "Settings.hpp"

class QComboBox;
class QSpinBox;

class RMS_UI : public AdditionalSettings
{
	Q_OBJECT
public:
	RMS_UI( RMS &block );

	void prepare();
private slots:
	void setValue( int v );
	void setMode( int m );
private:
	RMS &block;

	QComboBox *modeB;
	QSpinBox *msB;
};

#endif // RMS_HPP
