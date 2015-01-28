#ifndef ATLPT_SETTINGS_HPP
#define ATLPT_SETTINGS_HPP

#include "ui_AtLPT_Settings.h"

class QSettings;

class AtLPT_Settings : public QDialog
{
	Q_OBJECT
public:
	AtLPT_Settings( QSettings &settings );

	QAction *createAction();
private slots:
	void apply( QAbstractButton *b );
private:
	void reject();

	void loadValues();
	void setValues();

	Ui::AtLPT_Calibrate ui;
	QSettings &settings;
};

#endif // ATLPT_SETTINGS_HPP
