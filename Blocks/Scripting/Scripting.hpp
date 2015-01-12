#ifndef SCRIPTING_HPP
#define SCRIPTING_HPP

#include "Block.hpp"

#include <QMutex>

class Scripting : public Block
{
	friend class ScriptingUI;
public:
	inline Scripting( const QString &name, const QString &description, const QByteArray &code2 ) :
		Block( name, description, 1, 1, PROCESSING ),
		code2( code2 )
	{}
	virtual ~Scripting() {}
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	void setLabel();
protected:
	virtual bool compile( QString *errorStr = NULL ) = 0;

	QString generateOutArray() const;

	QByteArray label, code1, code2;
	QMutex mutex;
};

#include "Settings.hpp"

#include "SyntaxHighlighter.hpp"
#include "CodeEdit.hpp"

class QSyntaxHighlighter;
class QPushButton;
class QLineEdit;
class QSplitter;
class QLabel;

class ScriptingUI : public AdditionalSettings
{
	Q_OBJECT
public:
	ScriptingUI( Scripting &block, const QString &version = QString() );

	template< typename T > void setSyntaxHighlighter( SyntaxHighlighter::HighlightHints toHighlight, const QString &comment )
	{
		toHighlight += qMakePair
		(
			QStringList() << "SampleRate",
			SyntaxHighlighter::makeTxtChrFmt( Qt::darkGreen )
		);
		new T( code1E->document(), toHighlight, comment );

		toHighlight.last().first << "Out" << "In";
		new T( code2E->document(), toHighlight, comment );
	}

	void prepare();
	bool canClose();

	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );
private slots:
	void apply();
	void updateCurrentLine( int line );
private:
	void setTitle();

	QString initialWinTitle, version;
	QPoint winPos;

	CodeEdit *code1E, *code2E;
	QLineEdit *labelE;
	QPushButton *applyB;
	QLabel *infoL;

	QSplitter *splitter;

	Scripting &block;
};

#endif // SCRIPTING_HPP
