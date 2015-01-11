#include "Scripting.hpp"

void Scripting::serialize( QDataStream &ds ) const
{
	ds << label << code1 << code2;
	reinterpret_cast< ScriptingUI * >( settings->getAdditionalSettings() )->serialize( ds );
}
void Scripting::deSerialize( QDataStream &ds )
{
	ds >> label >> code1 >> code2;
	reinterpret_cast< ScriptingUI * >( settings->getAdditionalSettings() )->deSerialize( ds );
	setLabel();
}

void Scripting::setLabel()
{
	if ( label.isEmpty() )
		Block::label = getName();
	else
		Block::label = label;
	update();
}

QString Scripting::generateOutArray() const
{
	QString out_arr = "0";
	for ( int i = 1 ; i < outputsCount() ; ++i )
		out_arr += ",0";
	return out_arr;
}

#include <QPlainTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QLineEdit>
#include <QSplitter>
#include <QLayout>
#include <QLabel>

ScriptingUI::ScriptingUI( Scripting &block, const QString &version ) :
	AdditionalSettings( block ),
	block( block )
{
	labelE = new QLineEdit;
	labelE->setPlaceholderText( block.getName() );

	code1E = new QPlainTextEdit;
	code1E->setTabStopWidth( 20 );
	code1E->setFont( QFont( "Monospace" ) );
	code1E->setLineWrapMode( QPlainTextEdit::NoWrap );
	code1E->setWhatsThis( "Globalne zmienne i funkcje. Zawiera zmienne:\n  - SampleRate" );

	code2E = new QPlainTextEdit;
	code2E->setTabStopWidth( 20 );
	code2E->setFont( QFont( "Monospace" ) );
	code2E->setLineWrapMode( QPlainTextEdit::NoWrap );
	code2E->setWhatsThis( "Główna funkcja. Zawiera tablice:\n  - In,\n  - Out\nZwraca Out." );

	applyB = new QPushButton( "&Zastosuj" );
	applyB->setShortcut( QKeySequence( "Ctrl+S" ) );

	splitter = new QSplitter( Qt::Vertical );
	splitter->addWidget( code1E );
	splitter->addWidget( code2E );
	splitter->setSizes( QList< int >() << 1 << height() );

	QGridLayout *layout = new QGridLayout( this );
	layout->addWidget( splitter, 0, 0, 1, 3 );
	layout->addWidget( labelE, 1, 0, 1, 1 );
	layout->addWidget( applyB, 1, 1, 1, version.isEmpty() ? 2 : 1 );
	if ( !version.isEmpty() )
		layout->addWidget( new QLabel( version ), 1, 2, 1, 1 );
	layout->setMargin( 3 );

	connect( applyB, SIGNAL( clicked() ), this, SLOT( apply() ) );
}

void ScriptingUI::prepare()
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
bool ScriptingUI::canClose()
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

void ScriptingUI::serialize( QDataStream &ds ) const
{
	ds << parentWidget()->size() << splitter->saveState();
}
void ScriptingUI::deSerialize( QDataStream &ds )
{
	QSize parent_size;
	QByteArray splitter_state;
	ds >> parent_size >> splitter_state;
	parentWidget()->resize( parent_size );
	splitter->restoreState( splitter_state );
}

void ScriptingUI::apply()
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
		QString errorStr;

		block.code1 = code1E->toPlainText().toUtf8();
		block.code2 = code2E->toPlainText().toUtf8();

		block.mutex.lock();
		block.compile( &errorStr );
		block.mutex.unlock();

		code1E->document()->setModified( false );
		code2E->document()->setModified( false );

		if ( !errorStr.isEmpty() )
			QMessageBox::critical( this, block.getName(), errorStr );
	}
}

void ScriptingUI::setTitle()
{
	const QString label = labelE->text().replace( "\\n", " " );
	parentWidget()->setWindowTitle( initialWinTitle + ( !label.isEmpty() ? " (" + label + ")" : QString() ) );
}
