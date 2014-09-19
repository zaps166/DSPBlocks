#ifndef RTSETTINGS_HPP
#define RTSETTINGS_HPP

#include "ui_RTSettings.h"

class RTSettings : public QDialog
{
	Q_OBJECT
public:
	RTSettings( QWidget *parent );
private slots:
	void setPriorityRange( int sched );
	void accept();
private:
	Ui::RTSettings ui;
};

#endif // RTSETTINGS_HPP
