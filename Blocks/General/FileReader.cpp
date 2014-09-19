#include "FileReader.hpp"
#include "Array.hpp"

FileReader::FileReader() :
	Block( "File reader", "Odczytuje wartości próbek z pliku", 0, 1, SOURCE ),
	loop( true ), textMode( false ), pcm_s16( false )
{}

bool FileReader::start()
{
	if ( f.open( QFile::ReadOnly | QFile::OpenMode( textMode ? QFile::Text : 0 ) ) )
	{
		settings->setRunMode( true );
		return true;
	}
	err = "Nie można otworzyć pliku!";
	return false;
}
void FileReader::exec( Array< Sample > &samples )
{
	if ( !textMode )
	{
		QByteArray s;
		for ( int i = 0 ; i < outputsCount() ; ++i )
		{
			float sample = 0.0f;
			if ( !pcm_s16 )
			{
				s = f.read( sizeof( float ) );
				if ( s.size() == sizeof( float ) )
					sample = *( float * )s.data();
			}
			else
			{
				s = f.read( sizeof( qint16 ) );
				if ( s.size() == sizeof( qint16 ) )
					sample = *( qint16 * )s.data() / 32768.0f;
			}
			samples += ( Sample ){ getTarget( i ), sample };
		}
	}
	else
	{
		QStringList s_l = QString( f.readLine() ).simplified().split( ' ' );
		if ( s_l.count() == outputsCount() )
			for ( int i = 0 ; i < s_l.count() ; ++i )
				samples += ( Sample ){ getTarget( i ), s_l[ i ].toFloat() };
		else for ( int i = 0 ; i < outputsCount() ; ++i )
			samples += ( Sample ){ getTarget( i ), 0.0f };
	}
	if ( loop && f.atEnd() )
		f.reset();
}
void FileReader::stop()
{
	settings->setRunMode( false );
	f.close();
}

Block *FileReader::createInstance()
{
	FileReader *block = new FileReader;
	block->settings = new Settings( *block, false, 0, 0, true, 1, maxIO, false, new FileReaderUI( *block ) );
	return block;
}

void FileReader::serialize( QDataStream &ds ) const
{
	ds << f.fileName() << loop << textMode << pcm_s16;
}
void FileReader::deSerialize( QDataStream &ds )
{
	QString fPth;
	ds >> fPth >> loop >> textMode >> pcm_s16;
	f.setFileName( fPth );
}

#include <QToolButton>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QLayout>

FileReaderUI::FileReaderUI( FileReader &block ) :
	AdditionalSettings( block ),
	block( block )
{
	fileE = new QLineEdit;
	fileE->setPlaceholderText( "Ścieżka do pliku" );

	QToolButton *browseB = new QToolButton;
	browseB->setToolTip( "Przeglądaj" );
	browseB->setText( "..." );

	loopB = new  QCheckBox( "Zapętl odtwarzanie" );
	textModeB = new QCheckBox( "Plik tekstowy" );
	pcmB = new QCheckBox( "Odczyt liczb całkowitych 16bit" );

	applyB = new QPushButton( "&Zastosuj" );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( fileE, 0, 0, 1, 1 );
	layout->addWidget( browseB, 0, 1, 1, 1 );
	layout->addWidget( loopB, 1, 0, 1, 2 );
	layout->addWidget( textModeB, 2, 0, 1, 2 );
	layout->addWidget( pcmB, 3, 0, 1, 2 );
	layout->addWidget( applyB, 4, 0, 1, 2 );
	layout->setMargin( 3 );

	connect( loopB, SIGNAL( clicked() ), this, SLOT( setLoop() ) );
	connect( applyB, SIGNAL( clicked() ), this, SLOT( setFileName() ) );
	connect( browseB, SIGNAL( clicked() ), this, SLOT( browseFile() ) );
}

void FileReaderUI::prepare()
{
	fileE->setText( block.f.fileName() );
	loopB->setChecked( block.loop );
	pcmB->setChecked( block.pcm_s16 );
	connect( textModeB, SIGNAL( toggled( bool ) ), this, SLOT( setValue() ) );
	connect( pcmB, SIGNAL( toggled( bool ) ), this, SLOT( setValue() ) );
	textModeB->setChecked( block.textMode );
}
void FileReaderUI::setRunMode( bool b )
{
	fileE->setDisabled( b );
	textModeB->setDisabled( b );
	pcmB->setDisabled( b || textModeB->isChecked() );
	applyB->setDisabled( b );
}

void FileReaderUI::setValue()
{
	pcmB->setDisabled( block.textMode = textModeB->isChecked() );
	block.pcm_s16 = pcmB->isChecked();
}
void FileReaderUI::setLoop()
{
	block.loop = loopB->isChecked();
}
void FileReaderUI::setFileName()
{
	block.f.setFileName( fileE->text() );
}
void FileReaderUI::browseFile()
{
	QString newFile = QFileDialog::getOpenFileName( this, "Wybierz plik do odczytu", fileE->text(), QString(), NULL, Block::getNativeFileDialogFlag() );
	if ( !newFile.isEmpty() )
		fileE->setText( newFile );
}
