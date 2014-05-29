#include "WindFunc.hpp"

#include <QScriptEngine>

#define WindCosine( a ) "Math.cos( "a" * Math.PI * n / ( N - 1 ) )"

const QStringList WindFunc::windFunc = QStringList()
		<< "1.0"
		<< "0.53836 - 0.46164 * "WindCosine( "2.0" )
		<< "0.5 - 0.5 * "WindCosine( "2.0" )
		<< "( N - 1 ) / 2.0 - Math.abs( n - ( N - 1 ) / 2.0 )"
		<< "N / 2.0 - Math.abs( n - ( N - 1 ) / 2.0 )"
		<< "0.62 - 0.48 * Math.abs( n / ( N - 1 ) - 0.5 ) - 0.38 * "WindCosine( "2.0" )
		<< "0.42 - 0.5 * "WindCosine( "2.0" )" + 0.08 * "WindCosine( "4.0" )
		<< "0.355768 - 0.487396 * "WindCosine( "2.0" )" + 0.144232 * "WindCosine( "4.0" )" - 0.012604 * "WindCosine( "6.0" )
		<< "0.35875 - 0.48829 * "WindCosine( "2.0" )" + 0.14128 * "WindCosine( "4.0" )" - 0.01168 * "WindCosine( "6.0" )
		<< "0.3635819 - 0.4891775 * "WindCosine( "2.0" )" + 0.1365995 * "WindCosine( "4.0" )" - 0.0106411 * "WindCosine( "6.0" )
		<< "1.0 - 1.93 * "WindCosine( "2.0" )" + 1.29 * "WindCosine( "4.0" )" - 0.338 * "WindCosine( "6.0" )" + 0.032 * "WindCosine( "8.0" )
		   ;

QVector< double > WindFunc::windowFunctionCoefficient( const QString &code, int N, QString *err )
{
	QScriptEngine scriptE;
	scriptE.evaluate( "function genWindFunc( N ) { var windF = new Array( N ); for ( var n = 0 ; n < N ; ++n ) windF[ n ] = " + code + "; return windF; }" );
	QScriptValue genWindFunc = scriptE.globalObject().property( "genWindFunc" );
	QVariantList windF = genWindFunc.call( QScriptValue(), QScriptValueList() << N ).toVariant().toList();

	if ( err )
	{
		err->clear();
		if ( scriptE.hasUncaughtException() )
			*err = scriptE.uncaughtException().toString();
	}

	QVector< double > coeff;

	if ( windF.count() == N )
	{
		coeff.resize( N );
		for ( int i = 0 ; i < coeff.count() ; ++i )
			coeff[ i ] = windF[ i ].toDouble();
	}

	return coeff;
}

#include <QMessageBox>
#include <QLineEdit>
#include <QComboBox>
#include <QLayout>
#include <QLabel>

WindFunc::WindFunc( QWidget *parent ) :
	QFrame( parent )
{
	setFrameShape( StyledPanel );
	setFrameShadow( Raised );

	QLabel *windTypeL = new QLabel( "Okno" );

	windTypeCB = new QComboBox;
	windTypeCB->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
	windTypeCB->addItems( QStringList()
		<< "Prostokątne"
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
		<< "Użytkownika"
	);

	windFuncE = new QLineEdit;
	windFuncE->setToolTip( "Funkcja okna w języku JavaScript" );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( windTypeL, 0, 0, 1, 1 );
	layout->addWidget( windTypeCB, 0, 1, 1, 1 );
	layout->addWidget( windFuncE, 1, 0, 1, 2 );
	layout->setMargin( 3 );

	connect( windTypeCB, SIGNAL( currentIndexChanged( int ) ), this, SLOT( windTypeChanged( int ) ) );
	connect( windFuncE, SIGNAL( editingFinished() ), this, SLOT( setUserWindFunc() ) );

	windTypeCB->setCurrentIndex( Bartlett_Hanning );
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

QVector< double > WindFunc::getWindowFunctionCoefficient( int N )
{
	QString err;
	QVector< double > coeff = windowFunctionCoefficient( windFuncE->text(), N, &err );
	if ( !err.isEmpty() )
		QMessageBox::warning( this, "Generowanie funkcji okna", err );
	return coeff;
}

void WindFunc::windTypeChanged( int idx )
{
	bool isUserWindFunc = idx == windTypeCB->count() - 1;
	windFuncE->setText( isUserWindFunc ? userWindFunc : windFunc[ idx ] );
	windFuncE->setEnabled( isUserWindFunc );
}
void WindFunc::setUserWindFunc()
{
	userWindFunc = windFuncE->text();
}
