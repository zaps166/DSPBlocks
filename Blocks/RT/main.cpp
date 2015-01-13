class Block;

#ifdef USE_ATLPT
	#include "AtLPT_Out.hpp"
	#include "AtLPT_In.hpp"
#endif
#ifdef USE_COMEDI
	#include "ComediOut.hpp"
	#include "ComediIn.hpp"
#endif

#include <QList>

extern const char groupName[] = "Real-time";

extern "C" QList< Block * > createBlocks()
{
	return QList< Block * >()
#ifdef USE_ATLPT
		<< new AtLPT_Out
		<< new AtLPT_In
#endif
#ifdef USE_COMEDI
//		<< new ComediOut
//		<< new ComediIn
#endif
	;
}
