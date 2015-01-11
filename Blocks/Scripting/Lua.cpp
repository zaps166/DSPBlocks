#include "Lua.hpp"
#include "Global.hpp"
#include "Array.hpp"

#include <QMessageBox>
#include <QDebug>

#include <lua.hpp>

#if LUA_VERSION_NUM < 502
	#define lua_rawlen lua_objlen
#endif

Lua::Lua() :
	Block( "Lua", "Wprowadzanie kodu w języku Lua", 1, 1, PROCESSING ),
	code2( "for i = 1, math.min( #In, #Out ) do\n\tOut[ i ] = In[ i ]\nend\n" ),
	lua( NULL )
{}
Lua::~Lua()
{
	if ( lua )
		lua_close( lua );
}

bool Lua::start()
{
	settings->setRunMode( true );
	buffer.reset( new float[ inputsCount() ]() );
	return compile( false );
}
void Lua::setSample( int input, float sample )
{
	buffer[ input ] = sample;
}
void Lua::exec( Array< Sample > &samples )
{
	mutex.lock();
	if ( !err )
	{
		lua_getglobal( lua, "exec" );
		lua_createtable( lua, inputsCount(), 0 );
		for ( int i = 0 ; i < inputsCount() ; ++i )
		{
			lua_pushnumber( lua, buffer[ i ] );
			lua_rawseti( lua, -2, i+1 );
		}
		if ( !lua_pcall( lua, 1, 1, 0 ) )
		{
			if ( ( err = !lua_istable( lua, -1 ) ) )
				qDebug() << "Out must be array";
			else if ( !err && lua_rawlen( lua, -1 ) != outputsCount() )
			{
				qDebug() << "#Out !=" << outputsCount();
				err = true;
			}
		}
		else
		{
			qDebug() << lua_tostring( lua, -1 );
			lua_pop( lua, 1 );
			err = true;
		}
	}
	if ( !err )
	{
		for ( int i = 1 ; i <= outputsCount() ; ++i )
		{
			lua_rawgeti( lua, -i, i );
			samples += ( Sample ){ getTarget( i-1 ), ( float )lua_tonumber( lua, -1 ) };
		}
		lua_pop( lua, outputsCount() + 1 );
	}
	else for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), 0.0f };
	mutex.unlock();
}
void Lua::stop()
{
	settings->setRunMode( false );
	buffer.reset();
}

Block *Lua::createInstance()
{
	Lua *block = new Lua;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, false, new Lua_UI( *block ) );
	return block;
}

void Lua::serialize( QDataStream &ds ) const
{
	ds << label << code1 << code2;
	qobject_cast< Lua_UI * >( settings->getAdditionalSettings() )->serialize( ds );
}
void Lua::deSerialize( QDataStream &ds )
{
	ds >> label >> code1 >> code2;
	qobject_cast< Lua_UI * >( settings->getAdditionalSettings() )->deSerialize( ds );
	setLabel();
}

bool Lua::compile( bool showErr )
{
	if ( !lua )
	{
		lua = luaL_newstate();
		luaL_openlibs( lua );
	}

	QString out_arr = "0";
	for ( int i = 1 ; i < outputsCount() ; ++i )
		out_arr += ",0";

	const QString code =
		"local SampleRate = " + QString::number( Global::getSampleRate() ) + "\n"
		+ code1 + "\n" +
		"function exec( In )\n"
			"local Out = {" + out_arr + "}\n"
			+ code2 + "\n" +
			"return Out\n"
		"end";

	if ( luaL_loadstring( lua, code.toUtf8() ) || lua_pcall( lua, 0, 0, 0 ) )
	{
		if ( showErr )
			QMessageBox::warning( ( QWidget * )qApp->property( "MainWindow" ).value< void * >(), getName(), lua_tostring( lua, -1 ) );
		lua_pop( lua, 1 );
		err = true;
		return false;
	}

	err = false;
	return true;
}

void Lua::setLabel()
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
#include <QLabel>

Lua_UI::Lua_UI( Lua &block ) :
	AdditionalSettings( block ),
	block( block )
{
	labelE = new QLineEdit;
	labelE->setPlaceholderText( block.getName() );

	code1E = new QPlainTextEdit;
	code1E->setTabStopWidth( 20 );
	code1E->setFont( QFont( "Monospace" ) );
	code1E->setWhatsThis( "Globalne zmienne i funkcje. Zawiera zmienne:\n  - SampleRate" );

	code2E = new QPlainTextEdit;
	code2E->setTabStopWidth( 20 );
	code2E->setFont( QFont( "Monospace" ) );
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
	layout->addWidget( applyB, 1, 1, 1, 1 );
	layout->addWidget( new QLabel( LUA_VERSION ), 1, 2, 1, 1 );
	layout->setMargin( 3 );

	connect( applyB, SIGNAL( clicked() ), this, SLOT( apply() ) );
}

void Lua_UI::prepare()
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
bool Lua_UI::canClose()
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

void Lua_UI::serialize( QDataStream &ds ) const
{
	ds << parentWidget()->size() << splitter->saveState();
}
void Lua_UI::deSerialize( QDataStream &ds )
{
	QSize parent_size;
	QByteArray splitter_state;
	ds >> parent_size >> splitter_state;
	parentWidget()->resize( parent_size );
	splitter->restoreState( splitter_state );
}

void Lua_UI::apply()
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

void Lua_UI::setTitle()
{
	QString label = labelE->text().replace( "\\n", " " );
	parentWidget()->setWindowTitle( initialWinTitle + ( !label.isEmpty() ? " (" + label + ")" : QString() ) );
}
