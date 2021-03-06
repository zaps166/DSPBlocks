#include "Alsa.hpp"

typedef QPair< QStringList, QStringList > DevicesList;

static DevicesList getDevices( snd_pcm_stream_t stream_t )
{
	QPair< QStringList, QStringList > devices;

	snd_ctl_card_info_t *cardInfo;
	snd_ctl_card_info_alloca( &cardInfo );
	snd_pcm_info_t *pcmInfo;
	snd_pcm_info_alloca( &pcmInfo );
	int cardIdx = -1;
	while ( !snd_card_next( &cardIdx ) && cardIdx >= 0 )
	{
		const QString dev = "hw:" + QString::number( cardIdx );
		snd_ctl_t *ctl;
		if ( !snd_ctl_open( &ctl, dev.toLocal8Bit(), 0 ) )
		{
			if ( !snd_ctl_card_info( ctl, cardInfo ) )
			{
				const QString cardName = snd_ctl_card_info_get_name( cardInfo );
				int devIdx = -1;
				while ( !snd_ctl_pcm_next_device( ctl, &devIdx ) && devIdx >= 0 )
				{
					snd_pcm_info_set_device( pcmInfo, devIdx );
					snd_pcm_info_set_stream( pcmInfo, stream_t );
					if ( snd_ctl_pcm_info( ctl, pcmInfo ) >= 0 )
					{
						const QString pcmName = snd_pcm_info_get_name( pcmInfo );
						devices.first += dev + "," + QString::number( devIdx );
						devices.second += cardName + ( !pcmName.isEmpty() ? QString( ": " ) + snd_pcm_info_get_name( pcmInfo ) : QString() );
					}
				}
			}
			snd_ctl_close( ctl );
		}
	}

	char **hints;
	if ( !snd_device_name_hint( -1, "pcm", ( void *** )&hints ) )
	{
		char **n = hints;
		while ( *n != NULL )
		{
			char *name = snd_device_name_get_hint( *n, "NAME" );
			if ( name )
			{
				if ( strcmp( name, "null" ) )
				{
					char *colon = strchr( name, ':' );
					if ( colon )
						*colon = '\0';
					snd_pcm_t *snd = NULL;
					if ( !devices.first.contains( name ) && !snd_pcm_open( &snd, name, stream_t, 0 ) )
					{
						snd_pcm_close( snd );
						devices.first += name;
						char *desc = snd_device_name_get_hint( *n, "DESC" );
						if ( !desc )
							devices.second += QString();
						else
						{
							const QStringList descL = QString( desc ).split( ',' );
							devices.second += descL.count() > 1 ? descL[ 1 ].simplified() : descL[ 0 ].simplified();
							free( desc );
						}
					}
				}
				free( name );
			}
			++n;
		}
		snd_device_name_free_hint( ( void ** )hints );
	}

	return devices;
}

static QString getDeviceName( const DevicesList &devicesList, const QString &deviceName )
{
	int devIdx = devicesList.first.indexOf( deviceName );
	if ( devIdx < 0 )
	{
		devIdx = devicesList.first.indexOf( "default" );
		if ( devIdx < 0 )
			devIdx = devicesList.first.indexOf( "sysdefault" );
		if ( devIdx < 0 && !devicesList.first.isEmpty() )
			devIdx = 0;
	}
	return devIdx > -1 ? devicesList.first[ devIdx ] : QString();
}

/**/

Alsa::Alsa( const QString &name, const QString &description, int numInputs, int numOutputs, Type type ) :
	Block( name, description, numInputs, numOutputs, type ),
	snd( NULL ),
	devName( "default" )
{
	blocking = true;
}

void Alsa::serialize( QDataStream &ds ) const
{
	ds << devName;
}
void Alsa::deSerialize( QDataStream &ds )
{
	ds >> devName;
}

#include <QPushButton>
#include <QComboBox>
#include <QLayout>

AlsaUI::AlsaUI( Alsa &block ) :
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

void AlsaUI::prepare()
{
	DevicesList devicesList;
	switch ( block.getType() )
	{
		case Block::SINK:
			devicesList = getDevices( SND_PCM_STREAM_PLAYBACK );
			break;
		case Block::SOURCE:
			devicesList = getDevices( SND_PCM_STREAM_CAPTURE );
			break;
		default:
			break;
	}
	QString devName = getDeviceName( devicesList, block.devName );
	devicesCB->clear();
	for ( int i = 0 ; i < devicesList.first.count() ; ++i )
	{
		devicesCB->addItem( devicesList.second[ i ] + " (" + devicesList.first[ i ] + ")", devicesList.first[ i ] );
		if ( devName == devicesList.first[ i ] )
			devicesCB->setCurrentIndex( i );
	}
}
void AlsaUI::setRunMode( bool b )
{
	setDisabled( b );
}

void AlsaUI::setValue()
{
	if ( devicesCB->currentIndex() > -1 )
		block.devName = devicesCB->itemData( devicesCB->currentIndex() ).toString();
	else
		block.devName = "default";
}
