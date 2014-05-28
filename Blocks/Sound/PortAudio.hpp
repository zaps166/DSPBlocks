#ifndef PORTAUDIOCOMMON_HPP
#define PORTAUDIOCOMMON_HPP

#include <portaudio.h>

#include "Block.hpp"

class PortAudio : public Block
{
	friend class PortAudioUI;
public:
	PortAudio( const QString &name, const QString &description, int numInputs, int numOutputs, Type type );

	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );
private:
	QStringList getDeviceNames() const;
	int getDefaultDevice();
protected:
	int getDeviceIndex( const QString &name );

	bool err;
	QString devName;
	PaStream *stream;
};

#include "Settings.hpp"

class QComboBox;

class PortAudioUI : public AdditionalSettings
{
	Q_OBJECT
public:
	PortAudioUI( PortAudio &block );

	void prepare();
	void setRunMode( bool b );
private slots:
	void setValue();
private:
	PortAudio &block;

	QComboBox *devicesCB;
};

#endif // PORTAUDIOCOMMON_HPP
