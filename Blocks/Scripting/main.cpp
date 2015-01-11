#ifdef USE_JS
	#include "JS.hpp"
#endif
#ifdef USE_LUA
	#include "Lua.hpp"
#endif

#include <QList>

extern const char groupName[] = "Scripting";

extern "C" QList< Block * > createBlocks()
{
	return QList< Block * >()
#ifdef USE_JS
		<< new JS
#endif
#ifdef USE_LUA
		<< new Lua
#endif
	;
}
