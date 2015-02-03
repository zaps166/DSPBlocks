#include "AtLPT_Settings.hpp"
#include "AtLPT.hpp"

#include <QSettings>

AtLPT_Settings::AtLPT_Settings( QSettings &settings ) :
	settings( settings )
{
	ui.setupUi( this );

	loadValues();
	setValues();

	connect( ui.buttonBox, SIGNAL( clicked( QAbstractButton * ) ), this, SLOT( apply( QAbstractButton * ) ) );
}

QAction *AtLPT_Settings::createAction()
{
	QAction *act = new QAction( windowTitle(), this );
	connect( act, SIGNAL( triggered() ), this, SLOT( show() ) );
	return act;
}

void AtLPT_Settings::apply( QAbstractButton *b )
{
	if ( ui.buttonBox->buttonRole( b ) == QDialogButtonBox::AcceptRole || ui.buttonBox->buttonRole( b ) == QDialogButtonBox::ApplyRole )
	{
		settings.setValue( "AtLPT/InputOffset0", ui.inChn1B->value() );
		settings.setValue( "AtLPT/InputOffset1", ui.inChn2B->value() );

		settings.setValue( "AtLPT/OutputOffset0", ui.outChn1B->value() );
		settings.setValue( "AtLPT/OutputOffset1", ui.outChn2B->value() );

		if ( ui.voltageRangeB->isChecked() )
			settings.setValue( "AtLPT/Range", AtLPT::VoltageRange );
		else if ( ui.integerRangeB->isChecked() )
			settings.setValue( "AtLPT/Range", AtLPT::IntegerRange );
		else
			settings.setValue( "AtLPT/Range", AtLPT::StandardRange );

		setValues();
	}
}

void AtLPT_Settings::reject()
{
	loadValues();
	QDialog::reject();
}

void AtLPT_Settings::loadValues()
{
	ui.inChn1B->setValue( settings.value( "AtLPT/InputOffset0", AtLPT::inputOffset[ 0 ] ).toInt() );
	ui.inChn2B->setValue( settings.value( "AtLPT/InputOffset1", AtLPT::inputOffset[ 1 ] ).toInt() );

	ui.outChn1B->setValue( settings.value( "AtLPT/OutputOffset0", AtLPT::outputOffset[ 0 ] ).toInt() );
	ui.outChn2B->setValue( settings.value( "AtLPT/OutputOffset1", AtLPT::outputOffset[ 1 ] ).toInt() );

	switch ( settings.value( "AtLPT/Range", AtLPT::range ).toUInt() )
	{
		case AtLPT::VoltageRange:
			ui.voltageRangeB->setChecked( true );
			break;
		case AtLPT::IntegerRange:
			ui.integerRangeB->setChecked( true );
			break;
		case AtLPT::StandardRange:
		default:
			ui.standardRangeB->setChecked( true );
			break;
	}
}
void AtLPT_Settings::setValues()
{
	AtLPT::inputOffset[ 0 ] = ui.inChn1B->value();
	AtLPT::inputOffset[ 1 ] = ui.inChn2B->value();

	AtLPT::outputOffset[ 0 ] = ui.outChn1B->value();
	AtLPT::outputOffset[ 1 ] = ui.outChn2B->value();

	if ( ui.voltageRangeB->isChecked() )
		AtLPT::range = AtLPT::VoltageRange;
	else if ( ui.integerRangeB->isChecked() )
		AtLPT::range = AtLPT::IntegerRange;
	else
		AtLPT::range = AtLPT::StandardRange;
}
