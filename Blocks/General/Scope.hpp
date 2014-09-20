#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "Block.hpp"

#include <QWaitCondition>
#include <QCommonStyle>
#include <QThread>
#include <QMutex>

class Scope;

class DrawThr : public QThread
{
public:
	DrawThr( Scope &block );

	void start();
	void stop();
private:
	void run();

	Scope &block;

	volatile bool br;
};

class Scope : public Block, public QWidget
{
	friend class ScopeUI;
	friend class DrawThr;
public:
	Scope();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void inputsCountChanged( int num );

	void setBuffers();

	inline float getSample( int chn, int idx )
	{
		return idx < samplesVisible ? lastOutBuffer[ chn ][ idx ] : outBuffer[ chn ][ idx - samplesVisible ];
	}
	void draw();

	void paintEvent( QPaintEvent * );
	void closeEvent( QCloseEvent *event );
	void showEvent( QShowEvent *event );

	DrawThr drawThr;

	quint8 interp;
	int samplesVisible;
	float scale;
	bool xy, fallingSlope;
	int triggerChn;
	float triggerPos;

	QVector< QVector< float > > inBuffer, lastInBuffer, outBuffer, lastOutBuffer;
	QMutex execMutex, drawMutex, paintMutex;
	bool bufferReady, swapLastInBuffer;
	int buffPos, triggerHold;
	QWaitCondition drawCond;

	QVector< QPainterPath > paths;
	QCommonStyle style;
	QByteArray geo;
	bool cantClose;
};

#include "Settings.hpp"

class QDoubleSpinBox;
class QGroupBox;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QSlider;

class ScopeUI : public AdditionalSettings
{
	Q_OBJECT
public:
	ScopeUI( Scope &block );

	void prepare();

	void inputsCountChanged( int num );
private slots:
	void xyToggled( bool b );
	void setTrigger();
	void apply();
private:
	void checkXY( int numInputs );

	QComboBox *interpolationCB;
	QSpinBox *samplesVisibleB;
	QDoubleSpinBox *scaleB;
	QCheckBox *xyB;

	QGroupBox *triggerB;
	QComboBox *slopeCB;
	QSpinBox *trgChnB;
	QSlider *trgPosS;

	Scope &block;
};

#endif // SCOPE_HPP
