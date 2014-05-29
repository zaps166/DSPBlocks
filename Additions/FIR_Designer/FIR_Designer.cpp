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

	ui.srateB->setValue( Block::getSampleRate() );
	ui.cutoffB->setValue( ui.srateB->value() / 48 );

	ui.windFuncW->setUserWindFunc( settings.value( "FIR_Designer/UserWindFunc", "1.0" ).toString() );
	ui.windFuncW->setWindTypeIdx( settings.value( "FIR_Designer/WindTypeIdx", 0 ).toInt() );

	setFirCoeffE( NULL );
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

	double fc = ( double )ui.cutoffB->value() / ( double )ui.srateB->value();
	int M = ui.numCoeffB->value() - 1;
	QVector< double > coeff( M + 1 );
	bool high_pass = ui.highPassB->isChecked();

	QVector< double > wind_coeff = ui.windFuncW->getWindowFunctionCoefficient( coeff.count() );
	if ( wind_coeff.isEmpty() )
		return;

	for ( int i = 0 ; i < coeff.count() ; ++i )
	{
		if ( i == ( M >> 1 ) )
			coeff[ i ] = high_pass ? ( 1.0 - 2.0 * fc ) : ( 2.0 * fc );
		else
		{
			coeff[ i ] = sin( 2.0 * M_PI * fc * ( i - M / 2.0 ) ) * wind_coeff[ i ] / ( M_PI * ( i - M / 2.0 ) );
			if ( high_pass )
				coeff[ i ] = -coeff[ i ];
		}
	}

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
	ui.cutoffB->setMaximum( hz >> 1 );
}
