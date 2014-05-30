#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "RingBuffer.hpp"
#include "Block.hpp"

struct FFTContext;
struct FFTComplex;

#include <QMutex>

class Image : public Block
{
	friend class ImageUI;
public:
	Image();

	bool start();
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void outputsCountChanged( int num );

	bool fft_gen_img();
	bool xy_gen_img();
	void setLabel();

	RingBuffer< float > rdy_samples;
	QByteArray file;
	quint8 fftNbits;
	QMutex mutex;
};

#include "Settings.hpp"

class QLineEdit;
class QComboBox;
class QLabel;

class ImageUI : public AdditionalSettings
{
	Q_OBJECT
public:
	ImageUI( Image &block );

	void prepare();

	void itemsVisible();
private slots:
	void browseFile();
	void apply();
private:
	QLineEdit *fileE;
	QLabel *fftSizeL;
	QComboBox *fftSizeB;

	Image &block;
};

#endif // IMAGE_HPP
