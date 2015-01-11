#ifndef LUA_HPP
#define LUA_HPP

#include "Scripting.hpp"

struct lua_State;

class Lua : public Scripting
{
public:
	Lua();
	~Lua();

	bool start();
	void setSample( int input, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	bool compile( QString *errorStr = NULL );

	QScopedArrayPointer< float > buffer;
	lua_State *lua;
	bool err;
};

#endif
