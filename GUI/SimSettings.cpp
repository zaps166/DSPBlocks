#include "SimSettings.hpp"
#include "Block.hpp"

SimSettings::SimSettings( QWidget *parent, double simTime ) :
	QDialog( parent )
{
	ui.setupUi( this );

	ui.simTimeB->setValue( simTime );
	ui.srateB->setValue( Block::getSampleRate() );

	checkSampleRate();

	ui.refTimeB->setValue( Block::getRefTime() );

	connect( ui.srateB, SIGNAL( editingFinished() ), this, SLOT( checkSampleRate() ) );
}

void SimSettings::checkSampleRate()
{
	ui.refTimeB->setMaximum( qMin( ui.srateB->value(), 1000 ) );
}
