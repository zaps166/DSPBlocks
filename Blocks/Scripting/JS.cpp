#include "JS.hpp"
#include "Global.hpp"
#include "Array.hpp"

#include <QMessageBox>
#include <QDebug>

JS::JS() :
	Scripting( "JavaScript", "Wprowadzanie kodu w języku JavaScript", "for (i = 0; i < Math.min(Out.length, In.length); ++i)\n{\n\tOut[i] = In[i]\n}\n" ),
	err( true )
{}

bool JS::start()
{
	settings->setRunMode( true );

	buffer = scriptE.newArray( inputsCount() );
	for ( int i = 0 ; i < inputsCount() ; ++i )
		buffer.setProperty( i, 0.0f );

	return compile();
}
void JS::setSample( int input, float sample )
{
	buffer.setProperty( input, sample );
}
void JS::exec( Array< Sample > &samples )
{
	QVariantList Out;
	mutex.lock();
	if ( !err )
	{
		Out = execFunc.call( buffer ).toVariant().toList();
		if ( Out.count() != 2 )
		{
			if ( scriptE.hasUncaughtException() )
				qDebug() << scriptE.uncaughtException().toString();
			else
				qDebug() << "Out.length !=" << outputsCount();
			err = true;
		}
	}
	mutex.unlock();
	for ( int i = 0 ; i < outputsCount() ; ++i )
		samples += ( Sample ){ getTarget( i ), ( Out.count() > i ? Out[ i ].toFloat() : 0.0f ) };
}
void JS::stop()
{
	settings->setRunMode( false );
	buffer = QScriptValue();
	scriptE.collectGarbage();
}

Block *JS::createInstance()
{
	JS *block = new JS;
	block->settings = new Settings( *block, true, 1, maxIO, true, 1, maxIO, false, new ScriptingUI( *block ) );

	SyntaxHighlighter::HighlightHints toHighlight;
	toHighlight += qMakePair
	(
		QStringList() << "void" << "var" << "function",
		SyntaxHighlighter::makeTxtChrFmt( Qt::darkRed )
	);
	toHighlight += qMakePair
	(
		QStringList() << "break" << "case" << "catch" << "const" << "continue" << "debugger" << "default" << "delete" << "do" << "else" << "finally" << "for" << "if" << "in" << "instanceof" << "new" << "return" << "switch" << "this" << "throw" << "try" << "typeof" << "while" << "with",
		SyntaxHighlighter::makeTxtChrFmt( Qt::darkBlue )
	);
	toHighlight += qMakePair
	(
		QStringList() << "null" << "false" << "true" << "undefined",
		SyntaxHighlighter::makeTxtChrFmt( Qt::darkCyan )
	);
	block->settings->getAdditionalSettings< ScriptingUI >()->setSyntaxHighlighter< SyntaxHighlighter >( toHighlight, "//" );

	return block;
}

bool JS::compile( QString *errorStr )
{
	scriptE.evaluate
	(
		"var SampleRate = " + QString::number( Global::getSampleRate() ) + "\n"
		"var Out = [ " + generateOutArray() + " ]\n"
		+ code1 + "\n" +
		"function exec() {\n"
			"var In = this\n"
			+ code2 + "\n" +
			"return Out\n"
		"}"
	);
	if ( scriptE.hasUncaughtException() )
	{
		if ( errorStr )
			*errorStr = scriptE.uncaughtException().toString();
		execFunc = QScriptValue();
		err = true;
		return false;
	}
	execFunc = scriptE.globalObject().property( "exec" );
	err = false;
	return true;
}
