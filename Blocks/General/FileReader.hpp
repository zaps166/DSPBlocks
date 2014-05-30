#ifndef FILEREADER_HPP
#define FILEREADER_HPP

#include "Block.hpp"

#include <QFile>

class FileReader : public Block
{
	friend class FileReaderUI;
public:
	FileReader();

	bool start();
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	bool loop, textMode, pcm_s16;
	QFile f;
};

#include "Settings.hpp"

class QPushButton;
class QLineEdit;
class QCheckBox;

class FileReaderUI : public AdditionalSettings
{
	Q_OBJECT
public:
	FileReaderUI( FileReader &block );

	void prepare();
	void setRunMode( bool b );
private slots:
	void setValue();
	void setLoop();
	void setFileName();
	void browseFile();
private:
	FileReader &block;

	QLineEdit *fileE;
	QCheckBox *loopB, *textModeB, *pcmB;
	QPushButton *applyB;
};

#endif // FILEREADER_HPP
