#include "JS.hpp"
#include "Array.hpp"

JS::JS() :
	Block( "JavaScript", "Wprowadzanie kodu w języku JavaScript", 1, 1, PROCESSING ),
	code2( "output_samples = input_samples.slice();" )
{}

bool JS::start()
{
	settings->setRunMode( true );

	input_samples = scriptE.newArray( inputsCount() );
	for ( int i = 0 ; i < inputsCount() ; ++i )
		input_samples.setProperty( i, 0.0f );

	return compile();
}
void JS::setSample( int input, float sample )
{
	input_samples.setProperty( input, sample );
}
void JS::exec( Array< Sample > &samples )
{
	mutex.lock();
	QVariantList output_samples = mainFunc.call( input_samples ).toVariant().toList();
	mutex.unlock();
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), ( output_samples.count() > i ? output_samples[ i ].toFloat() : 0.0f ) };
}
void JS::stop()
{
	settings->setRunMode( false );
}

Block *JS::createInstance()
{
	JS *block = new JS;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, false, new JS_UI( *block ) );
	return block;
}

void JS::serialize( QDataStream &ds ) const
{
	ds << label << code1 << code2;
	qobject_cast< JS_UI * >( settings->getAdditionalSettings() )->serialize( ds );
}
void JS::deSerialize( QDataStream &ds )
{
	ds >> label >> code1 >> code2;
	qobject_cast< JS_UI * >( settings->getAdditionalSettings() )->deSerialize( ds );
	setLabel();
}

bool JS::compile()
{
	scriptE.evaluate
	(
		"var sample_rate = " + QString::number( getSampleRate() ) + ";"
		"var inputs_count = " + QString::number( inputsCount() ) + ";"
		"var outputs_count = " + QString::number( outputsCount() ) + ";"
		+ code1 +
		"function main() {"
			"var input_samples = this;"
			"var output_samples = new Array( outputs_count );"
			+ code2 +
			"return output_samples;"
		"}"
	);
	mainFunc = scriptE.globalObject().property( "main" );
	return true;
}

void JS::setLabel()
{
	if ( label.isEmpty() )
		Block::label = getName();
	else
		Block::label = label;
	update();
}

#include <QPlainTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QLineEdit>
#include <QSplitter>
#include <QLayout>

JS_UI::JS_UI( JS &block ) :
	AdditionalSettings( block ),
	block( block )
{
	labelE = new QLineEdit;
	labelE->setPlaceholderText( block.getName() );

	code1E = new QPlainTextEdit;
	code1E->setFont( QFont( "Monospace" ) );
	code1E->setWhatsThis( "Globalne zmienne i funkcje. Zawiera:\n  - sample_rate,\n  - inputs_count,\n  - outputs_count" );

	code2E = new QPlainTextEdit;
	code2E->setFont( QFont( "Monospace" ) );
	code2E->setWhatsThis( "Główna funkcja. Zawiera tablice:\n  - input_samples,\n  - output_samples\nZwraca output_samples." );

	applyB = new QPushButton( "&Zastosuj" );
	applyB->setShortcut( QKeySequence( "Ctrl+S" ) );

	splitter = new QSplitter( Qt::Vertical );
	splitter->addWidget( code1E );
	splitter->addWidget( code2E );
	splitter->setSizes( QList< int >() << height() / 3 << height() - height() / 3 );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( labelE );
	layout->addWidget( splitter );
	layout->addWidget( applyB );
	layout->setMargin( 3 );

	connect( applyB, SIGNAL( clicked() ), this, SLOT( apply() ) );
}

void JS_UI::prepare()
{
	QString label = block.label;
	label.replace( "\n", "\\n" );
	labelE->setText( label );
	code1E->setPlainText( block.code1 );
	code2E->setPlainText( block.code2 );
	code2E->setFocus();
	if ( initialWinTitle.isNull() )
		initialWinTitle = parentWidget()->windowTitle();
	setTitle();
	if ( !winPos.isNull() )
		parentWidget()->move( winPos );
}
bool JS_UI::canClose()
{
	if ( code1E->document()->isModified() || code2E->document()->isModified() )
		switch ( QMessageBox::question( this, block.getName(), "Czy chcesz zastosować zmiany?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel ) )
		{
			case QMessageBox::Yes:
				apply();
				break;
			case QMessageBox::No:
				break;
			default:
				return false;
		}
	code1E->clear();
	code2E->clear();
	AdditionalSettings::canClose();
	winPos = parentWidget()->pos();
	return true;
}

void JS_UI::serialize( QDataStream &ds ) const
{
	ds << parentWidget()->size() << splitter->saveState();
}
void JS_UI::deSerialize( QDataStream &ds )
{
	QSize parent_size;
	QByteArray splitter_state;
	ds >> parent_size >> splitter_state;
	parentWidget()->resize( parent_size );
	splitter->restoreState( splitter_state );
}

void JS_UI::apply()
{
	if ( labelE->text().isEmpty() || labelE->text() == block.getName() )
		block.label.clear();
	else
	{
		block.label = labelE->text().toUtf8();
		block.label.replace( "\\n", "\n" );
	}
	setTitle();
	block.setLabel();
	if ( code1E->document()->isModified() || code2E->document()->isModified() )
	{
		block.code1 = code1E->toPlainText().toUtf8();
		block.code2 = code2E->toPlainText().toUtf8();

		block.mutex.lock();
		block.compile();
		block.mutex.unlock();

		code1E->document()->setModified( false );
		code2E->document()->setModified( false );
	}
}

void JS_UI::setTitle()
{
	QString label = labelE->text().replace( "\\n", " " );
	parentWidget()->setWindowTitle( initialWinTitle + ( !label.isEmpty() ? " (" + label + ")" : QString() ) );
}
