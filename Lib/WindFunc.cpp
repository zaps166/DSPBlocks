#include "WindFunc.hpp"
#include "GraphW.hpp"

#include <QScriptEngine>

#define WindCosine( a ) "Math.cos( "a" * Math.PI * n / ( N - 1.0 ) )"

const QStringList WindFunc::windFunc = QStringList()
		<< "1.0"
		<< "1.0 - Math.pow( ( n - ( N - 1 ) / 2.0 ) / ( ( N + 1.0 ) / 2 ), 2.0 )"
		<< "0.53836 - 0.46164 * "WindCosine( "2.0" )
		<< "0.5 * ( 1.0 - "WindCosine( "2.0" )" )"
		<< "1.0 - Math.abs( ( n - ( N - 1 ) / 2.0 ) / ( ( N - 1 ) / 2.0 ) )"
		<< "1.0 - Math.abs( ( n - ( N - 1 ) / 2.0 ) / ( N / 2.0 ) )"
		<< "0.62 - 0.48 * Math.abs( n / ( N - 1 ) - 0.5 ) - 0.38 * "WindCosine( "2.0" )
		<< "0.42 - 0.5 * "WindCosine( "2.0" )" + 0.08 * "WindCosine( "4.0" )
		<< "0.355768 - 0.487396 * "WindCosine( "2.0" )" + 0.144232 * "WindCosine( "4.0" )" - 0.012604 * "WindCosine( "6.0" )
		<< "0.35875 - 0.48829 * "WindCosine( "2.0" )" + 0.14128 * "WindCosine( "4.0" )" - 0.01168 * "WindCosine( "6.0" )
		<< "0.3635819 - 0.4891775 * "WindCosine( "2.0" )" + 0.1365995 * "WindCosine( "4.0" )" - 0.0106411 * "WindCosine( "6.0" )
		<< "1.0 - 1.93 * "WindCosine( "2.0" )" + 1.29 * "WindCosine( "4.0" )" - 0.338 * "WindCosine( "6.0" )" + 0.032 * "WindCosine( "8.0" );

QVector< double > WindFunc::windowFunctionCoefficients( const QString &code, int N, QString *err )
{
	int halfN = N / 2;

	QScriptEngine scriptE;
	scriptE.evaluate
	(
		"function genWindFunc( N, halfN ) {"
			"var windF = new Array( halfN + 1 );"
			"for ( var n = 0 ; n <= halfN ; ++n )"
			"windF[ n ] = " + code + ";"
			"return windF;"
		"}"
	);
	QScriptValue genWindFunc = scriptE.globalObject().property( "genWindFunc" );
	QVariantList windF = genWindFunc.call( QScriptValue(), QScriptValueList() << N << halfN ).toVariant().toList();

	if ( err )
	{
		err->clear();
		if ( scriptE.hasUncaughtException() )
			*err = scriptE.uncaughtException().toString();
	}

	QVector< double > coeff;

	if ( windF.count() == halfN + 1 )
	{
		coeff.resize( N );
		for ( int n = 0 ; n <= halfN ; ++n )
			coeff[ n ] = coeff[ N - 1 - n ] = windF[ n ].toDouble();
	}

	return coeff;
}

static double modZeroBessel( double x )
{
	double x_2 = x / 2.0;
	double num = 1.0, fact = 1.0, result = 1.0;
	for ( int i = 1 ; i < 20 ; ++i )
	{
		num *= x_2 * x_2;
		fact *= i;
		result += num / ( fact * fact );
	}
	return result;
}

#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLayout>
#include <QLabel>

#include <math.h>

WindFunc::WindFunc( QWidget *parent ) :
	QFrame( parent ),
	kaiserEditEnabled( true ),
	kaiserBeta( 1.0 )
{
	setFrameShape( StyledPanel );
	setFrameShadow( Raised );

	QLabel *windTypeL = new QLabel( "Okno" );

	windTypeCB = new QComboBox;
	windTypeCB->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
	windTypeCB->addItems( QStringList()
		<< "Prostokątne"
		<< "Welcha"
		<< "Hamminga"
		<< "Hanninga"
		<< "Bartletta"
		<< "Trójkątne"
		<< "Bartletta-Hanna"
		<< "Blackmana"
		<< "Nuttalla"
		<< "Blackmana-Harrisa"
		<< "Blackmana-Nuttalla"
		<< "Flat top"
		<< "Kaisera"
		<< "Użytkownika"
	);

	showGraphB = new QPushButton( "Pokaż wykres" );

	windFuncE = new QLineEdit;

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( windTypeL, 0, 0, 1, 1 );
	layout->addWidget( windTypeCB, 0, 1, 1, 1 );
	layout->addWidget( showGraphB, 0, 2, 1, 1 );
	layout->addWidget( windFuncE, 1, 0, 1, 3 );
	layout->setMargin( 3 );

	connect( windTypeCB, SIGNAL( currentIndexChanged( int ) ), this, SLOT( windFuncChanged( int ) ) );
	connect( windFuncE, SIGNAL( editingFinished() ), this, SLOT( setUserWindFunc() ) );
	connect( showGraphB, SIGNAL( clicked() ), this, SLOT( showGraph() ) );

	windTypeCB->setCurrentIndex( Hamming );
}

int WindFunc::getWindTypeIdx() const
{
	return windTypeCB->currentIndex();
}
void WindFunc::setWindTypeIdx( int idx )
{
	if ( idx > -1 && idx < windTypeCB->count() )
		windTypeCB->setCurrentIndex( idx );
}

void WindFunc::setKaiserEditEnabled( bool e )
{
	kaiserEditEnabled = e;
	if ( windTypeCB->currentIndex() == Kaiser )
		windFuncE->setEnabled( kaiserEditEnabled );
}
void WindFunc::setKaiserBeta( double beta )
{
	kaiserBeta = beta;
	if ( windTypeCB->currentIndex() == Kaiser )
		windFuncE->setText( QString::number( kaiserBeta ) );
}

QVector< double > WindFunc::getWindowFunctionCoefficient( int N )
{
	QVector< double > coeff;
	if ( windTypeCB->currentIndex() == Kaiser )
	{
		double halfM = ( N - 1 ) / 2.0;
		double denom = modZeroBessel( kaiserBeta );
		coeff.resize( N );
		for ( int n = 0 ; n < N ; ++n )
		{
			double val = ( n - halfM ) / halfM;
			val = 1.0 - ( val * val );
			coeff[ n ] = modZeroBessel( kaiserBeta * sqrt( val ) ) / denom;
		}
	}
	else
	{
		QString err;
		coeff = windowFunctionCoefficients( windFuncE->text(), N, &err );
		if ( !err.isEmpty() )
			QMessageBox::warning( this, "Generowanie funkcji okna", err );
	}
	return coeff;
}

void WindFunc::windFuncChanged( int idx )
{
	bool isUserWindFunc = idx == windTypeCB->count() - 1;
	bool isKaiserWindFunc = idx == Kaiser;
	windFuncE->setText( isUserWindFunc ? userWindFunc : ( isKaiserWindFunc ? QString::number( kaiserBeta ) : windFunc[ idx ] ) );
	windFuncE->setEnabled( isUserWindFunc || ( isKaiserWindFunc && kaiserEditEnabled ) );
	windFuncE->setToolTip( isKaiserWindFunc ? "Współczynnik beta" : "Funkcja okna w języku JavaScript" );
	emit windFuncChanged( isKaiserWindFunc );
}
void WindFunc::setUserWindFunc()
{
	bool isKaiser = windTypeCB->currentIndex() == Kaiser;
	if ( isKaiser )
		kaiserBeta = windFuncE->text().toDouble();
	else
		userWindFunc = windFuncE->text();
	emit windFuncChanged( isKaiser );
}
void WindFunc::showGraph()
{
	QVector< double > window = getWindowFunctionCoefficient( 1000 );
	if ( !window.isEmpty() )
	{
		GraphW *graphW = new GraphW( this );
		graphW->setWindowFlags( Qt::Window );
		graphW->setAttribute( Qt::WA_DeleteOnClose );
		graphW->setWindowTitle( "Okno " + windTypeCB->currentText() );
		graphW->set_y_samples( window );
		graphW->show();
	}
}
