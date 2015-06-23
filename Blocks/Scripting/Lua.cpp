#include "Lua.hpp"
#include "Global.hpp"
#include "Array.hpp"

#include <QMessageBox>
#include <QDebug>

extern "C"
{
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

#if LUA_VERSION_NUM < 502
	#define lua_rawlen lua_objlen
#endif

Lua::Lua() :
	Scripting( "Lua", "Wprowadzanie kodu w języku Lua", "for i=1, math.min(#In, #Out) do\n\tOut[i] = In[i]\nend\n" ),
	lua( NULL ),
	err( true )
{}
Lua::~Lua()
{
	if ( lua )
		lua_close( lua );
}

bool Lua::start()
{
	settings->setRunMode( true );
	buffer.reset( new float[ inputsCount() ]() );
	return compile();
}
void Lua::setSample( int input, float sample )
{
	buffer[ input ] = sample;
}
void Lua::exec( Array< Sample > &samples )
{
	mutex.lock();
	if ( !err )
	{
		lua_getglobal( lua, "exec" );
		lua_createtable( lua, inputsCount(), 0 );
		for ( int i = 0 ; i < inputsCount() ; ++i )
		{
			lua_pushnumber( lua, buffer[ i ] );
			lua_rawseti( lua, -2, i+1 );
		}
		if ( !lua_pcall( lua, 1, 1, 0 ) )
		{
			if ( ( err = !lua_istable( lua, -1 ) ) )
				qDebug() << "Out must be array";
			else if ( !err && lua_rawlen( lua, -1 ) != outputsCount() )
			{
				qDebug() << "#Out !=" << outputsCount();
				err = true;
			}
		}
		else
		{
			qDebug() << lua_tostring( lua, -1 );
			lua_pop( lua, 1 );
			err = true;
		}
	}
	if ( !err )
	{
		for ( int i = 1 ; i <= outputsCount() ; ++i )
		{
			lua_rawgeti( lua, -i, i );
			samples += ( Sample ){ getTarget( i-1 ), ( float )lua_tonumber( lua, -1 ) };
		}
		lua_pop( lua, outputsCount() + 1 );
	}
	else for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), 0.0f };
	mutex.unlock();
}
void Lua::stop()
{
	settings->setRunMode( false );
	buffer.reset();
}

Block *Lua::createInstance()
{
	Lua *block = new Lua;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, false, new ScriptingUI( *block, LUA_VERSION ) );

	SyntaxHighlighter::HighlightHints toHighlight;
	toHighlight += qMakePair
	(
		QStringList() << "and" << "function" << "in" << "local" << "not" << "or",
		SyntaxHighlighter::makeTxtChrFmt( Qt::darkRed )
	);
	toHighlight += qMakePair
	(
		QStringList() << "break" << "do" << "else" << "elseif" << "end" << "for" << "if" << "repeat" << "return" << "then" << "until" << "while",
		SyntaxHighlighter::makeTxtChrFmt( Qt::darkBlue )
	);
	toHighlight += qMakePair
	(
		QStringList() << "nil" << "false" << "true",
		SyntaxHighlighter::makeTxtChrFmt( Qt::darkCyan )
	);
	block->settings->getAdditionalSettings< ScriptingUI >()->setSyntaxHighlighter< SyntaxHighlighter >( toHighlight, "--" );

	return block;
}

bool Lua::compile( QString *errorStr )
{
	if ( !lua )
	{
		lua = luaL_newstate();
		luaL_openlibs( lua );
	}
	const QString code =
		"local SampleRate = " + QString::number( Global::getSampleRate() ) + "\n"
		"local Out = {" + generateOutArray() + "}\n"
		+ code1 + "\n" +
		"function exec( In )\n"
			+ code2 + "\n" +
			"return Out\n"
		"end";
	if ( luaL_loadstring( lua, code.toUtf8() ) || lua_pcall( lua, 0, 0, 0 ) )
	{
		if ( errorStr )
			*errorStr = lua_tostring( lua, -1 );
		lua_pop( lua, 1 );
		err = true;
		return false;
	}
	err = false;
	return true;
}
