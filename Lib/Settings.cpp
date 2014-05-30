#include "Settings.hpp"
#include "Block.hpp"

#include <QApplication>
#include <QCloseEvent>
#include <QPushButton>
#include <QSpinBox>
#include <QLayout>
#include <QLabel>

AdditionalSettings::AdditionalSettings( Block &block ) :
	block( block )
{
	setFrameShape( QFrame::StyledPanel );
	setFrameShadow( QFrame::Sunken );
}

bool AdditionalSettings::canClose()
{
	block.emitSaveState();
	return true;
}

/**/

Settings::IO::IO( const QString &labelTxt, int min, int max )
{
	label = new QLabel( labelTxt );
	label->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Fixed );

	num = new QSpinBox;
	num->setRange( min, max );

	QHBoxLayout *layout = new QHBoxLayout( this );
	layout->setMargin( 0 );
	layout->addWidget( label );
	layout->addWidget( num );
}

void Settings::IO::connectValueChanged( QObject *obj, const char *slot )
{
	connect( num, SIGNAL( valueChanged( int ) ), obj, slot );
}

/**/

Settings::Settings( Block &block, bool canModifyInputs, quint8 inputsMin, quint8 inputsMax, bool canModifyOutputs, quint8 outputsMin, quint8 outputsMax, bool independent, AdditionalSettings *additionalSettings ) :
	block( block ),
	additionalSettings( additionalSettings )
{
	setParent( ( QWidget * )qApp->property( "MainWindow" ).value< void * >() );
	setWindowTitle( "Ustawienia \"" + block.getName() + '"' );
	setWindowFlags( Qt::Dialog );

	QVBoxLayout *layout = new QVBoxLayout( this );
	layout->setMargin( 3 );

	if ( additionalSettings )
		layout->addWidget( additionalSettings );

	if ( !canModifyInputs )
		inputs = NULL;
	else
	{
		inputs = new IO( "Liczba wejść: ", inputsMin, inputsMax );
		inputs->connectValueChanged( this, SLOT( inputsCountChanged( int ) ) );
		layout->addWidget( inputs );
	}
	if ( !canModifyOutputs )
		outputs = NULL;
	else
	{
		outputs = new IO( "Liczba wyjść: ", outputsMin, outputsMax );
		outputs->connectValueChanged( this, SLOT( outputsCountChanged( int ) ) );
		layout->addWidget( outputs );
	}

	if ( independent && inputs && outputs )
	{
		connect( inputs->num, SIGNAL( valueChanged( int ) ), outputs->num, SLOT( setValue( int ) ) );
		connect( outputs->num, SIGNAL( valueChanged( int ) ), inputs->num, SLOT( setValue( int ) ) );
	}

	closeB = new QPushButton( "Zamknij" );
	closeB->setShortcut( QKeySequence( Qt::Key_Escape ) );
	connect( closeB, SIGNAL( clicked() ), this, SLOT( close() ) );
	layout->addWidget( closeB );
}

void Settings::prepare()
{
	if ( additionalSettings && !additionalSettings->isVisible() )
		additionalSettings->prepare();
	if ( inputs )
		inputs->num->setValue( block.inputsCount() );
	if ( outputs )
		outputs->num->setValue( block.outputsCount() );
}
void Settings::setRunMode( bool b )
{
	if ( additionalSettings )
		additionalSettings->setRunMode( b );
	if ( inputs )
		inputs->setDisabled( b );
	if ( outputs )
		outputs->setDisabled( b );
}

void Settings::inputsCountChanged( int num )
{
	block.setInputsCount( num );
}
void Settings::outputsCountChanged( int num )
{
	block.setOutputsCount( num );
}

void Settings::closeEvent( QCloseEvent *event )
{
	if ( additionalSettings && !additionalSettings->canClose() )
		event->ignore();
	else
		QWidget::closeEvent( event );
}
