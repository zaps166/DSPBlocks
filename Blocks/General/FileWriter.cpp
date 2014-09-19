#include "FileWriter.hpp"

FileWriter::FileWriter() :
	Block( "File writer", "Zapisuje wartości próbek do pliku", 1, 0, SINK ),
	textMode( false ), pcm_s16( false ), saveIfDiff( false )
{}

bool FileWriter::start()
{
	if ( f.open( QFile::WriteOnly | QFile::OpenMode( textMode ? QFile::Text : 0 ) ) )
	{
		settings->setRunMode( true );
		buffer.resize( inputsCount() );
		return true;
	}
	return false;
}
void FileWriter::setSample( int input, float sample )
{
	buffer[ input ] = sample;
}
void FileWriter::exec( Array< Sample > & )
{
	if ( !saveIfDiff || buffer != lastBuffer )
	{
		if ( !textMode )
		{
			if ( !pcm_s16 )
				f.write( ( char * )buffer.data(), buffer.size() * sizeof( float ) );
			else for ( int i = 0 ; i < buffer.count() ; ++i )
			{
				qint16 s = buffer[ i ] * 32767.0;
				f.write( ( char * )&s, sizeof s );
			}
		}
		else
		{
			for ( int i = 0 ; i < inputsCount() ; ++i )
				f.write( QString( i > 0 ? " %1" : "%1" ).arg( buffer[ i ], 0, 'f', 5 ).toLatin1() );
			f.putChar( '\n' );
		}
		lastBuffer = buffer;
	}
}
void FileWriter::stop()
{
	settings->setRunMode( false );
	lastBuffer.clear();
	buffer.clear();
	f.close();
}

Block *FileWriter::createInstance()
{
	FileWriter *block = new FileWriter;
	block->settings = new Settings( *block, true, 1, maxIO, false, 0, 0, false, new FileWriterUI( *block ) );
	return block;
}

void FileWriter::serialize( QDataStream &ds ) const
{
	ds << f.fileName() << textMode << pcm_s16 << saveIfDiff;
}
void FileWriter::deSerialize( QDataStream &ds )
{
	QString fPth;
	ds >> fPth >> textMode >> pcm_s16 >> saveIfDiff;
	f.setFileName( fPth );
}

#include <QToolButton>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QLayout>

FileWriterUI::FileWriterUI( FileWriter &block ) :
	AdditionalSettings( block ),
	block( block )
{
	fileE = new QLineEdit;
	fileE->setPlaceholderText( "Ścieżka do pliku" );

	QToolButton *browseB = new QToolButton;
	browseB->setToolTip( "Przeglądaj" );
	browseB->setText( "..." );

	textModeB = new QCheckBox( "Plik tekstowy" );
	pcmB = new QCheckBox( "Zapis liczb całkowitych 16bit" );
	saveIfDiff = new QCheckBox( "Zapis, jeżeli różne" );

	applyB = new QPushButton( "&Zastosuj" );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( fileE, 0, 0, 1, 1 );
	layout->addWidget( browseB, 0, 1, 1, 1 );
	layout->addWidget( textModeB, 1, 0, 1, 2 );
	layout->addWidget( pcmB, 2, 0, 1, 2 );
	layout->addWidget( pcmB, 3, 0, 1, 2 );
	layout->addWidget( saveIfDiff, 4, 0, 1, 2 );
	layout->addWidget( applyB, 5, 0, 1, 2 );
	layout->setMargin( 3 );

	connect( applyB, SIGNAL( clicked() ), this, SLOT( setFileName() ) );
	connect( browseB, SIGNAL( clicked() ), this, SLOT( browseFile() ) );
}

void FileWriterUI::prepare()
{
	fileE->setText( block.f.fileName() );
	pcmB->setChecked( block.pcm_s16 );
	connect( textModeB, SIGNAL( toggled( bool ) ), this, SLOT( setValue() ) );
	connect( pcmB, SIGNAL( toggled( bool ) ), this, SLOT( setValue() ) );
	textModeB->setChecked( block.textMode );
	saveIfDiff->setChecked( block.saveIfDiff );
	connect( saveIfDiff, SIGNAL( toggled( bool ) ), this, SLOT( setValue() ) );
}
void FileWriterUI::setRunMode( bool b )
{
	fileE->setDisabled( b );
	textModeB->setDisabled( b );
	pcmB->setDisabled( b || textModeB->isChecked() );
	saveIfDiff->setDisabled( b );
	applyB->setDisabled( b );
}

void FileWriterUI::setValue()
{
	pcmB->setDisabled( block.textMode = textModeB->isChecked() );
	block.pcm_s16 = pcmB->isChecked();
	block.saveIfDiff = saveIfDiff->isChecked();
}
void FileWriterUI::setFileName()
{
	block.f.setFileName( fileE->text() );
}
void FileWriterUI::browseFile()
{
	QString newFile = QFileDialog::getSaveFileName( this, "Wybierz plik zapisu", fileE->text(), QString(), NULL, Block::getNativeFileDialogFlag() );
	if ( !newFile.isEmpty() )
		fileE->setText( newFile );
}
