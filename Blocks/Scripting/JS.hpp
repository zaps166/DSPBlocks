#ifndef JS_HPP
#define JS_HPP

#include "Block.hpp"

#include <QScriptEngine>
#include <QMutex>

class JS : public Block
{
	friend class JS_UI;
public:
	JS();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	bool compile( bool showErr = true );

	void setLabel();

	QByteArray label, code1, code2;

	QScriptEngine scriptE;
	QScriptValue mainFunc, buffer;

	QMutex mutex;
};

#include "Settings.hpp"

class QPlainTextEdit;
class QPushButton;
class QLineEdit;
class QSplitter;

class JS_UI : public AdditionalSettings
{
	Q_OBJECT
public:
	JS_UI( JS &block );

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

	JS &block;
};

#endif // JS_HPP
