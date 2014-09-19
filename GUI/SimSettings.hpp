#ifndef SIMSETTINGS_HPP
#define SIMSETTINGS_HPP

#include "ui_SimSettings.h"

class SimSettings : public QDialog
{
	Q_OBJECT
public:
	SimSettings( QWidget *parent, double simTime );

	inline double getSimulationTime() const
	{
		return ui.simTimeB->value();
	}
	inline int getSampleRate() const
	{
		return ui.srateB->value();
	}
	inline int getRefTime() const
	{
		return ui.refTimeB->value();
	}
private slots:
	void checkSampleRate();
private:
	Ui::SimSettings ui;
};

#endif // SIMSETTINGS_HPP
