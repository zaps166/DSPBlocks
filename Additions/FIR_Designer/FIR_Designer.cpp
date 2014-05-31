#include "FIR_Designer.hpp"
#include "Block.hpp"

#include <QPlainTextEdit>
#include <QClipboard>
#include <QSettings>
#include <QDebug>

#include <math.h>

FIR_Designer::FIR_Designer( QSettings &settings ) :
	settings( settings )
{
	ui.setupUi( this );
	ui.rippleB->hide(); //TODO - Kaiser window

	ui.srateB->setValue( Block::getSampleRate() );
	ui.cutoffB->setValue( ui.srateB->value() / 48 );
	ui.cutF2B->setValue( ui.srateB->value() / 24 );
	ui.cutF1B->setValue( ui.srateB->value() / 48 );

	ui.windFuncW->setUserWindFunc( settings.value( "FIR_Designer/UserWindFunc", "1.0" ).toString() );
	ui.windFuncW->setWindTypeIdx( settings.value( "FIR_Designer/WindTypeIdx", 0 ).toInt() );

	setFirCoeffE( NULL );
	updateFreqs();
}
FIR_Designer::~FIR_Designer()
{
	settings.setValue( "FIR_Designer/UserWindFunc", ui.windFuncW->getUserWindFunc() );
	settings.setValue( "FIR_Designer/WindTypeIdx", ui.windFuncW->getWindTypeIdx() );
}

QAction *FIR_Designer::createAction()
{
	QAction *act = new QAction( windowTitle(), this );
	connect( act, SIGNAL( triggered() ), this, SLOT( show() ) );
	return act;
}

void FIR_Designer::on_textEditChooseB_clicked( bool b )
{
	if ( !b )
	{
		setCursor( Qt::ArrowCursor );
		releaseMouse();
		if ( firCoeffE )
			genCoeffsIfCan();
	}
	else
	{
		setFirCoeffE( NULL );
		setCursor( Qt::CrossCursor );
		grabMouse();
	}
}
void FIR_Designer::on_textEditB_toggled( bool b )
{
	if ( !b )
		setFirCoeffE( NULL );
}
void FIR_Designer::on_coeffGenB_clicked()
{
	if ( firCoeffE && !qApp->allWidgets().contains( firCoeffE ) )
		setFirCoeffE( NULL );

	int M = ui.numCoeffB->value() - 1;

	QVector< double > coeff( M + 1 );

	QVector< double > wind_coeff = ui.windFuncW->getWindowFunctionCoefficient( coeff.count() );
	if ( wind_coeff.isEmpty() )
		return;

	double halfM = M / 2.0;
	int halfLen = coeff.count() / 2;

	if ( ui.lowPassB->isChecked() || ui.highPassB->isChecked() )
	{
		double fc = ( double )ui.cutoffB->value() / ( double )ui.srateB->value();
		if ( halfLen * 2 != coeff.count() )
		{
			double val = 2.0 * fc;
			if ( ui.highPassB->isChecked() )
				val = 1.0 - val;
			coeff[ halfLen ] = val;
		}
		if ( ui.highPassB->isChecked() )
			fc = -fc;
		for ( int n = 0 ; n < halfLen ; ++n )
			coeff[ n ] = coeff[ coeff.count() - 1 - n ] = sin( 2.0 * M_PI * fc * ( n - halfM ) ) / ( M_PI * ( n - halfM ) );
	}
	else if ( ui.bandPassB->isChecked() || ui.bandStopB->isChecked() )
	{
		double fc1 = ( double )ui.cutF1B->value() / ( double )ui.srateB->value();
		double fc2 = ( double )ui.cutF2B->value() / ( double )ui.srateB->value();
		if ( halfLen * 2 != coeff.count() )
		{
			double val = 2.0 * ( fc2 - fc1 );
			if ( ui.bandStopB->isChecked() )
				val = 1.0 - val;
			coeff[ halfLen ] = val;
		}
		if ( ui.bandStopB->isChecked() )
			qSwap( fc1, fc2 );
		for ( int n = 0 ; n < halfLen ; ++n )
		{
			double val1 = sin( 2.0 * M_PI * fc1 * ( n - halfM ) ) / ( M_PI * ( n - halfM ) );
			double val2 = sin( 2.0 * M_PI * fc2 * ( n - halfM ) ) / ( M_PI * ( n - halfM ) );
			coeff[ n ] = coeff[ coeff.count() - 1 - n ] = val2 - val1;
		}
	}

	for ( int i = 0 ; i < coeff.count() ; ++i )
		coeff[ i ] *= wind_coeff[ i ];

	ui.graphW->set_y_samples( coeff );

	if ( firCoeffE || ui.toClipboardB->isChecked() )
	{
		QString coeff_str;
		int precission = ui.precissionB->value();
		for ( int i = 0 ; i < coeff.count() ; ++i )
			coeff_str += QString::number( coeff[ i ], 'g', precission ) + "\n";
		coeff_str.chop( 1 );

		if ( firCoeffE )
			firCoeffE->setPlainText( coeff_str );

		if ( ui.toClipboardB->isChecked() )
			qApp->clipboard()->setText( coeff_str );
	}
}
void FIR_Designer::on_liveUpdateB_clicked( bool b )
{
	ui.coeffGenB->setDisabled( b );
	if ( b )
		on_coeffGenB_clicked();
}
void FIR_Designer::on_toClipboardB_clicked( bool b )
{
	if ( b )
		genCoeffsIfCan();
}
void FIR_Designer::on_numCoeffB_editingFinished()
{
	/* Filtry inne niż dolnoprzepustowy muszą mieć nieparzystą liczbę współczynników */
	int numCoeff = ui.numCoeffB->value();
	if ( !( numCoeff & 1 ) && !ui.lowPassB->isChecked() )
		ui.numCoeffB->setValue( numCoeff + 1 );
	genCoeffsIfCan();
}
void FIR_Designer::on_cutF1B_valueChanged( int hz )
{
	ui.cutF2B->setMinimum( hz );
	genCoeffsIfCan();
}
void FIR_Designer::on_cutF2B_valueChanged( int hz )
{
	ui.cutF1B->setMaximum( hz );
	if ( hz < ui.cutF2B->value() )
		ui.cutF2B->setValue( ui.cutF1B->value() );
	genCoeffsIfCan();
}

void FIR_Designer::updateFreqs()
{
	if ( ui.lowPassB->isChecked() || ui.highPassB->isChecked() )
	{
		ui.bandFreqsB->hide();
		ui.cutoffB->show();
	}
	else if ( ui.bandPassB->isChecked() || ui.bandStopB->isChecked() )
	{
		ui.bandFreqsB->show();
		ui.cutoffB->hide();
	}
	on_numCoeffB_editingFinished();
	genCoeffsIfCan();
}
void FIR_Designer::genCoeffsIfCan()
{
	if ( ui.liveUpdateB->isChecked() )
		on_coeffGenB_clicked();
}

void FIR_Designer::mousePressEvent( QMouseEvent *e )
{
	if ( !ui.textEditChooseB->isChecked() )
		QWidget::mouseMoveEvent( e );
	else
	{
		QWidget *w = qApp->widgetAt( QCursor::pos() );
		if ( w && w->parentWidget() )
			setFirCoeffE( qobject_cast< QPlainTextEdit * >( w->parentWidget() ) );
		ui.textEditChooseB->click();
	}
}
void FIR_Designer::closeEvent( QCloseEvent *e )
{
	if ( ui.textEditChooseB->isChecked() )
		ui.textEditChooseB->click();
	else
		setFirCoeffE( NULL );
	QWidget::closeEvent( e );
}

void FIR_Designer::setFirCoeffE( QPlainTextEdit *e )
{
	firCoeffE = e;
	ui.textEditChoosedL->setText( firCoeffE ? "tak" : "nie" );
}

void FIR_Designer::on_srateB_valueChanged( int hz )
{
	ui.cutoffB->setMaximum( hz / 2 );
	ui.cutF2B->setMaximum( hz / 2 );
	genCoeffsIfCan();
}
