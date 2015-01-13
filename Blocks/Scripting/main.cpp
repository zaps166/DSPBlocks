#include "JS.hpp"
#ifdef USE_LUA
	#include "Lua.hpp"
#endif

#include <QList>

extern const char groupName[] = "Scripting";

extern "C" QList< Block * > createBlocks()
{
	return QList< Block * >()
		<< new JS
#ifdef USE_LUA
		<< new Lua
#endif
	;
}
