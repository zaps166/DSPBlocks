#ifndef SPECTRUM_HPP
#define SPECTRUM_HPP

#include "Block.hpp"

#include <QCommonStyle>
#include <QMutex>

struct FFTContext;
struct FFTComplex;

class Spectrum : public Block, public QWidget
{
	friend class SpectrumUI;
public:
	Spectrum();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void setFFT();
	void freeFFT();
	void setLabel();

	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void paintEvent( QPaintEvent * );
	void closeEvent( QCloseEvent *event );
	void showEvent( QShowEvent *event );
	void mouseMoveEvent( QMouseEvent *event );

	int pos;
	FFTContext *fftCtx;
	FFTComplex **fftCplx;

	bool spectogram;
	int fftSize, numSpectrums;

	QVector< QVector< float > > spectrum;
	QImage spectogramImg;

	QByteArray geo;
	QCommonStyle style;
	QLinearGradient linearGrad;
	QMutex mutex;
	bool cantClose;
	float spectrumScale;
};

#include "Settings.hpp"

class QDoubleSpinBox;
class QCheckBox;
class QComboBox;
class QSpinBox;

class SpectrumUI : public AdditionalSettings
{
	Q_OBJECT
public:
	SpectrumUI( Spectrum &block );

	void prepare();
private slots:
	void setValue();
private:
	QComboBox *fftSizeB;
	QCheckBox *spectogramB;
	QSpinBox *numSpectrumsB;
	QDoubleSpinBox *scaleB;

	Spectrum &block;
};

#endif // SPECTRUM_HPP
