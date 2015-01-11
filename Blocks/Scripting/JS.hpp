#ifndef JS_HPP
#define JS_HPP

#include "Scripting.hpp"

#include <QScriptEngine>

class JS : public Scripting
{
public:
	JS();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	bool compile( QString *errorStr = NULL );

	QScriptValue execFunc, buffer;
	QScriptEngine scriptE;
	bool err;
};

#endif // JS_HPP
