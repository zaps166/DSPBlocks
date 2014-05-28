#ifndef ALSACOMMON_HPP
#define ALSACOMMON_HPP

#include "Block.hpp"

#include <alsa/asoundlib.h>

#include <QDebug>

class Alsa : public Block
{
	friend class AlsaUI;
public:
	Alsa( const QString &name, const QString &description, int numInputs, int numOutputs, Type type );

	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );
protected:
	bool err;
	snd_pcm_t *snd;
	QString devName;
};

#include "Settings.hpp"

class QComboBox;

class AlsaUI : public AdditionalSettings
{
	Q_OBJECT
public:
	AlsaUI( Alsa &block );

	void prepare();
	void setRunMode( bool b );
private slots:
	void setValue();
private:
	Alsa &block;

	QComboBox *devicesCB;
};

#endif // ALSACOMMON_HPP
