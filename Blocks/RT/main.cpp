#include "AtLPT_Out.hpp"
#include "AtLPT_In.hpp"

#include <QList>

extern const char groupName[] = "Real-time";

extern "C" QList< Block * > createBlocks()
{
	return QList< Block * >()
		<< new AtLPT_In
		<< new AtLPT_Out
	;
}
