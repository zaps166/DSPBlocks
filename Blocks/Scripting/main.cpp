#include "JS.hpp"
#include "Lua.hpp"

#include <QList>

extern const char groupName[] = "Scripting";

extern "C" QList< Block * > createBlocks()
{
	return QList< Block * >() << new JS << new Lua;
}
