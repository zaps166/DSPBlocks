#include "RTSettings.hpp"
#include "Global.hpp"

#include <QThread>

RTSettings::RTSettings( QWidget *parent ) :
	QDialog( parent )
{
	ui.setupUi( this );

	ui.groupBox->setChecked( Global::isRealTime() );

	ui.schedB->setCurrentIndex( Global::getSched() );
	setPriorityRange( ui.schedB->currentIndex() );
	ui.priorityB->setValue( Global::getPriority() );

	ui.cpuB->setMaximum( QThread::idealThreadCount() );
	ui.cpuB->setValue( Global::getCPU() );

	switch ( Global::getRtMode() )
	{
		case Global::NANOSLEEP:
			ui.nanosleepB->setChecked( true );
			break;
		case Global::CLOCK_NANOSLEEP:
		default:
			ui.clockNanosleepB->setChecked( true );
			break;
	}
}

void RTSettings::setPriorityRange( int sched )
{
	if ( sched == SCHED_OTHER )
		ui.priorityB->setRange( 0, 0 );
	else
	{
		bool priority0 = ui.priorityB->value() == 0;
		ui.priorityB->setRange( 1, 99 );
		if ( priority0 )
			ui.priorityB->setValue( DEFAULT_PRIORITY );
	}
}
void RTSettings::accept()
{
	int rt_mode = Global::CLOCK_NANOSLEEP;
	if ( ui.nanosleepB->isChecked() )
		rt_mode = Global::NANOSLEEP;
	Global::setRealTime( ui.groupBox->isChecked(), ui.cpuB->value(), ui.schedB->currentIndex(), ui.priorityB->value(), rt_mode );
	QDialog::accept();
}
