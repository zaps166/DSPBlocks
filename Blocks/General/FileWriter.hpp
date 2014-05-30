#ifndef FILEWRITER_HPP
#define FILEWRITER_HPP

#include "Block.hpp"

#include <QFile>

class FileWriter : public Block
{
	friend class FileWriterUI;
public:
	FileWriter();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	QVector< float > buffer, lastBuffer;
	bool textMode, pcm_s16, saveIfDiff;
	QFile f;
};

#include "Settings.hpp"

class QPushButton;
class QLineEdit;
class QCheckBox;

class FileWriterUI : public AdditionalSettings
{
	Q_OBJECT
public:
	FileWriterUI( FileWriter &block );

	void prepare();
	void setRunMode( bool b );
private slots:
	void setValue();
	void setFileName();
	void browseFile();
private:
	FileWriter &block;

	QLineEdit *fileE;
	QCheckBox *textModeB, *pcmB, *saveIfDiff;
	QPushButton *applyB;
};

#endif // FILEWRITER_HPP
