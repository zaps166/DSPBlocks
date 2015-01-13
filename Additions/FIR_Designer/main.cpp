#include "FIR_Designer.hpp"

class QSettings;

extern "C" QList< QAction * >getActions( QSettings &settings )
{
	return QList< QAction * >()
		<< ( new FIR_Designer( settings ) )->createAction();
}
