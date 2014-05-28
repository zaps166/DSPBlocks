#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "Block.hpp"

#include <QCommonStyle>
#include <QMutex>

class Scope : public Block, public QWidget
{
	friend class ScopeUI;
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

	void setBuffer();
	void setSamples( int from );

	void paintEvent( QPaintEvent * );
	void closeEvent( QCloseEvent *event );
	void showEvent( QShowEvent *event );

	int samplesVisible;
	float scale;
	bool xy, fallingSlope;
	int triggerChn;
	float triggerPos;

	QVector< QVector< float > > buffer;
	int buffPos, triggerHold;

	QVector< QPainterPath > paths;
	QCommonStyle style;
	QByteArray geo;
	bool cantClose;
	QMutex mutex;
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
