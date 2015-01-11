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

class QPlainTextEdit;
class QPushButton;
class QLineEdit;
class QSplitter;

class ScriptingUI : public AdditionalSettings
{
	Q_OBJECT
public:
	ScriptingUI( Scripting &block, const QString &version = QString() );

	void prepare();
	bool canClose();

	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );
private slots:
	void apply();
private:
	void setTitle();

	QString initialWinTitle;
	QPoint winPos;

	QLineEdit *labelE;

	QPlainTextEdit *code1E, *code2E;
	QPushButton *applyB;

	QSplitter *splitter;

	Scripting &block;
};

#endif // SCRIPTING_HPP
