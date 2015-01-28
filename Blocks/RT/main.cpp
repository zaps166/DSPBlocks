class QSettings;
class Block;

#ifdef USE_ATLPT
	#include "AtLPT_Settings.hpp"
	#include "AtLPT_Out.hpp"
	#include "AtLPT_In.hpp"
#endif
#ifdef USE_COMEDI
	#include "ComediOut.hpp"
	#include "ComediIn.hpp"
#endif

#include <QList>

extern const char groupName[] = "Real-Time";

extern "C" QList< Block * > createBlocks()
{
	return QList< Block * >()
#ifdef USE_ATLPT
		<< new AtLPT_Out
		<< new AtLPT_In
#endif
#ifdef USE_COMEDI
		<< new ComediOut
		<< new ComediIn
#endif
	;
}

#ifdef USE_ATLPT
extern "C" QList< QAction * >getActions( QSettings &settings )
{
	return QList< QAction * >()
		<< ( new AtLPT_Settings( settings ) )->createAction();
}
#endif
