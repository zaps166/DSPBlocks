#ifndef FIR_DESIGNER_HPP
#define FIR_DESIGNER_HPP

#include "ui_FIR_Designer.h"

class QPlainTextEdit;
class QSettings;

class FIR_Designer : public QWidget
{
	Q_OBJECT
public:
	FIR_Designer( QSettings &settings );
	~FIR_Designer();

	QAction *createAction();
private slots:
	void on_textEditChooseB_clicked( bool b );
	void on_srateB_valueChanged( int hz );
	void on_textEditB_toggled( bool b );
	void on_coeffGenB_clicked();
private:
	void mousePressEvent( QMouseEvent *e );
	void closeEvent( QCloseEvent *e );

	void setFirCoeffE( QPlainTextEdit *e );

	Ui::FIR_Designer ui;
	QSettings &settings;

	QPlainTextEdit *firCoeffE;
};

#endif // FIR_DESIGNER_HPP
