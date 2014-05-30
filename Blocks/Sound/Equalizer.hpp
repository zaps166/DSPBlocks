#ifndef EQUALIZER_HPP
#define EQUALIZER_HPP

#include "Block.hpp"

struct FFTContext;
struct FFTComplex;

#include <QMutex>

class Equalizer : public Block
{
	friend class EqualizerUI;
public:
	Equalizer();

	bool start();
	void setSample( int in_buffer, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void fft_start();
	void fft_free();

	quint8 fftNbits;
	int fftSize, fftSize_2, pos;

	QVector< QVector< float > > in_buffer, rdy_samples;
	FFTContext *fftIn, *fftOut;
	QVector< float > wind_f, f;
	FFTComplex *fftCplx;

	QMutex mutex;
};

#include "Settings.hpp"

class QComboBox;

class EqualizerUI : public AdditionalSettings
{
	Q_OBJECT
public:
	EqualizerUI( Equalizer &block );

	void prepare();
private slots:
	void setValue();
private:
	QComboBox *fftSizeB;

	Equalizer &block;
};

#endif // EQUALIZER_HPP
