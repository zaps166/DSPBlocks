#ifndef LUA_HPP
#define LUA_HPP

#include "Block.hpp"

#include <QMutex>

struct lua_State;

class Lua : public Block
{
	friend class Lua_UI;
public:
	Lua();
	~Lua();

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

	QScopedArrayPointer< float > buffer;
	QByteArray label, code1, code2;
	lua_State *lua;
	bool err;

	QMutex mutex;
};

#include "Settings.hpp"

class QPlainTextEdit;
class QPushButton;
class QLineEdit;
class QSplitter;

class Lua_UI : public AdditionalSettings
{
	Q_OBJECT
public:
	Lua_UI( Lua &block );

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

	Lua &block;
};

#endif // LUA_HPP
