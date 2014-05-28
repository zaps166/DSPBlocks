#include "PortAudio.hpp"

#define getDeviceName QString( hostApiInfo->name ) + ": " + QString::fromLocal8Bit( deviceInfo->name )

PortAudio::PortAudio( const QString &name, const QString &description, int numInputs, int numOutputs, Type type ) :
	Block( name, description, numInputs, numOutputs, type ),
	err( false ),
	stream( NULL )
{
	blocking = true;
}

void PortAudio::serialize( QDataStream &ds ) const
{
	ds << devName;
}
void PortAudio::deSerialize( QDataStream &ds )
{
	ds >> devName;
}

QStringList PortAudio::getDeviceNames() const
{
	QStringList deviceNames;
	int numDevices = Pa_GetDeviceCount();
	for ( int i = 0 ; i < numDevices ; ++i )
	{
		const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo( i );
		if ( deviceInfo )
		{
			const PaHostApiInfo *hostApiInfo = Pa_GetHostApiInfo( deviceInfo->hostApi );
			switch ( getType() )
			{
				case Block::SINK:
					if ( deviceInfo->maxOutputChannels > 0 )
						deviceNames += getDeviceName;
					break;
				case Block::SOURCE:
					if ( deviceInfo->maxInputChannels > 0 )
						deviceNames += getDeviceName;
					break;
				default:
					break;
			}
		}
	}
	return deviceNames;
}
int PortAudio::getDeviceIndex( const QString &name )
{
	int numDevices = Pa_GetDeviceCount();
	for ( int i = 0 ; i < numDevices ; ++i )
	{
		const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo( i );
		if ( deviceInfo )
		{
			const PaHostApiInfo *hostApiInfo = Pa_GetHostApiInfo( deviceInfo->hostApi );
			if ( name == getDeviceName )
				switch ( getType() )
				{
					case Block::SINK:
						if ( deviceInfo->maxOutputChannels > 0 )
							return i;
						break;
					case Block::SOURCE:
						if ( deviceInfo->maxInputChannels > 0 )
							return i;
						break;
					default:
						break;
				}
		}
	}
	return getDefaultDevice();
}
int PortAudio::getDefaultDevice()
{
#ifdef Q_OS_LINUX
	if ( getDeviceNames().contains( "ALSA: default" ) )
		return getDeviceIndex( "ALSA: default" );
#endif
	switch ( getType() )
	{
		case Block::SINK:
			return Pa_GetDefaultOutputDevice();
		case Block::SOURCE:
			return Pa_GetDefaultInputDevice();
		default:
			return -1;
	}
}

#include <QPushButton>
#include <QComboBox>
#include <QLayout>

PortAudioUI::PortAudioUI( PortAudio &block ) :
	AdditionalSettings( block ),
	block( block )
{
	devicesCB = new QComboBox;

	QPushButton *applyB = new QPushButton( "&Zastosuj" );

	QVBoxLayout *layout = new QVBoxLayout( this );
	layout->addWidget( devicesCB );
	layout->addWidget( applyB );
	layout->setMargin( 3 );

	connect( applyB, SIGNAL( clicked() ), this, SLOT( setValue() ) );
}

void PortAudioUI::prepare()
{
	devicesCB->clear();
	devicesCB->addItems( QStringList( "Domyślne" ) + block.getDeviceNames() );
	int idx = devicesCB->findText( block.devName );
	devicesCB->setCurrentIndex( idx < 0 ? 0 : idx );
}
void PortAudioUI::setRunMode( bool b )
{
	setDisabled( b );
}

void PortAudioUI::setValue()
{
	block.devName = devicesCB->currentIndex() == 0 ? QString() : devicesCB->currentText();
}

