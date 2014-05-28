#include "Block.hpp"

#include <QList>

extern const char groupName[] = "RT";

extern "C" QList< Block * > createBlocks()
{
	return QList< Block * >();
}
