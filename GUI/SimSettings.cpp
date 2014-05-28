#include "SimSettings.hpp"

SimSettings::SimSettings( QWidget *parent, double simTime, int srate, int refTime ) :
	QDialog( parent )
{
	ui.setupUi( this );

	ui.simTimeB->setValue( simTime );
	ui.srateB->setValue( srate );

	checkSampleRate();

	ui.refTimeB->setValue( refTime );

	connect( ui.srateB, SIGNAL( editingFinished() ), this, SLOT( checkSampleRate() ) );
}

void SimSettings::checkSampleRate()
{
	ui.refTimeB->setMaximum( qMin( ui.srateB->value(), 1000 ) );
}
