#include "FIR_Designer.hpp"
#include "Global.hpp"

#include <QPlainTextEdit>
#include <QMessageBox>
#include <QClipboard>
#include <QSettings>
#include <QDebug>

#include <math.h>

FIR_Designer::FIR_Designer( QSettings &settings ) :
	settings( settings )
{
	ui.setupUi( this );

	ui.windFuncW->setKaiserEditEnabled( false );

	ui.srateB->setValue( Global::getSampleRate() );
	on_srateB_editingFinished();
	ui.cutoffB->setValue( ui.srateB->value() / 48 );
	ui.cutF2B->setValue( ui.srateB->value() / 24 );
	ui.cutF1B->setValue( ui.srateB->value() / 48 );

	setFirCoeffE( NULL );
	updateFreqs();

	QString userWindFunc = settings.value( "FIR_Designer/UserWindFunc" ).toString();
	int windTypeIdx = settings.value( "FIR_Designer/WindTypeIdx", -1 ).toInt();

	if ( !userWindFunc.isEmpty() )
		ui.windFuncW->setUserWindFunc( userWindFunc );
	if ( windTypeIdx > -1 )
		ui.windFuncW->setWindTypeIdx( windTypeIdx );
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
void FIR_Designer::on_srateB_editingFinished()
{
	ui.cutoffB->setMaximum( ui.srateB->value() / 2 );
	ui.cutF2B->setMaximum( ui.srateB->value() / 2 );
	if ( ui.transitionWidthB->isEnabled() )
		calcKaiserBeta();
	genCoeffsIfCan();
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
	ui.cutF2B->setMinimum( hz+1 );
	genCoeffsIfCan();
}
void FIR_Designer::on_cutF2B_valueChanged( int hz )
{
	ui.cutF1B->setMaximum( hz-1 );
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
}
void FIR_Designer::calcKaiserBeta()
{
	double dw = 2.0 * M_PI * ui.transitionWidthB->value() / ui.srateB->value(); // Calculate delta w
	double a = -20.0 * log10( ui.rippleB->value() ); // Calculate ripple dB
	int m = a > 21 ? ceil( ( a - 7.95 ) / ( 2.285 * dw ) ) : ceil( 5.79 / dw ); // Calculate filter order
	if ( a <= 21 )
		ui.windFuncW->setKaiserBeta( 0.0 );
	else if ( a <= 50 )
		ui.windFuncW->setKaiserBeta( 0.5842 * pow( a - 21.0, 0.4 ) + 0.07886 * ( a - 21.0 ) );
	else
		ui.windFuncW->setKaiserBeta( 0.1102 * ( a - 8.7 ) );
	int numCoeff = m + 1;
	if ( numCoeff > ui.numCoeffB->maximum() )
		QMessageBox::information( this, "Okno Kaisera", "Za duża liczba współczynników, filtr będzie nieprawidłowy.\nZmień parametry!" );
	ui.numCoeffB->setValue( m + 1 );
	on_numCoeffB_editingFinished(); /* Żeby była nieparzysta liczba współczynników wtedy, kiedy trzeba */
}
void FIR_Designer::genCoeffsIfCan()
{
	if ( ui.liveUpdateB->isChecked() )
		on_coeffGenB_clicked();
}
void FIR_Designer::windFuncChanged( bool isKaiser )
{
	ui.rippleB->setEnabled( isKaiser );
	ui.transitionWidthB->setEnabled( isKaiser );
	ui.numCoeffB->setDisabled( isKaiser );
	if ( isKaiser )
		calcKaiserBeta();
	else
		genCoeffsIfCan();
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
