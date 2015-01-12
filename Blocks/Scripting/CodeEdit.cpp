#include "CodeEdit.hpp"

#include <QTextBlock>
#include <QKeyEvent>

CodeEdit::CodeEdit() :
	lastBlockCount( document()->blockCount() )
{
	connect( document(), SIGNAL( blockCountChanged( int ) ), this, SLOT( blockCountChanged( int ) ) );
	connect( this, SIGNAL( cursorPositionChanged() ), this, SLOT( cursorPositionChanged() ) );
}

void CodeEdit::blockCountChanged( int blockCount ) //wcięcia
{
	QTextCursor txtCursor = textCursor();
	if ( lastBlockCount < blockCount && !txtCursor.positionInBlock() ) //została utworzona nowa linijka
	{
		const int line = txtCursor.block().firstLineNumber();
		if ( line > 0 ) //jest poprzednia linijka
		{
			const QString prevBlock = document()->findBlockByLineNumber( line - 1 ).text();
			for ( int i = 0 ; i < prevBlock.length() ; ++i ) //utrzymywanie wcięć
			{
				if ( prevBlock.at( i ) == '\t' )
					txtCursor.insertText( "\t" );
				else if ( prevBlock.at( i ) == ' ' )
					txtCursor.insertText( " " );
				else
					break;
			}
		}
	}
	lastBlockCount = blockCount;
}
void CodeEdit::cursorPositionChanged() //kolorowanie aktualnego wiersza
{
	ExtraSelection currentLine;
	if ( textInteractionFlags() != Qt::NoTextInteraction )
	{
		currentLine.format.setBackground( QColor( 0xE0EAFA ) );
		currentLine.format.setProperty( QTextFormat::FullWidthSelection, true );
		currentLine.cursor = textCursor();
		currentLine.cursor.clearSelection();
		emit lineNumber( textCursor().block().firstLineNumber() + 1 );
	}
	setExtraSelections( QList< ExtraSelection >() << currentLine );
}

void CodeEdit::focusOutEvent( QFocusEvent *e )
{
	QTextEdit::focusOutEvent( e );
	setExtraSelections( QList< ExtraSelection >() );
}
void CodeEdit::focusInEvent( QFocusEvent *e )
{
	QTextEdit::focusInEvent( e );
	cursorPositionChanged();
}
void CodeEdit::keyPressEvent( QKeyEvent *e )
{
	if ( e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab )
	{
		QTextCursor txtCursor = textCursor();
		const QTextDocument *doc = document();
		const QTextBlock selectionEndBlock = doc->findBlock( txtCursor.selectionEnd() );
		int start_bolck_line = doc->findBlock( txtCursor.selectionStart() ).firstLineNumber();
		int end_bolck_line = selectionEndBlock.firstLineNumber();
		if ( start_bolck_line != end_bolck_line )
		{
			if ( txtCursor.selectionEnd() == selectionEndBlock.position() ) //nie robi wcięcia na linijce, która nie ma nic zaznaczone
				--end_bolck_line;
			txtCursor.beginEditBlock();
			for ( int i = start_bolck_line ; i <= end_bolck_line ; ++i )
			{
				const QTextBlock block = doc->findBlockByLineNumber( i );
				txtCursor.setPosition( block.position() );
				if ( e->key() == Qt::Key_Tab )
					txtCursor.insertText( "\t" );
				else
				{
					const char c = block.text().at( txtCursor.positionInBlock() ).toLatin1();
					if ( c == '\t' || c == ' ' )
						txtCursor.deleteChar();
				}
			}
			txtCursor.endEditBlock();
			return;
		}
	}
	QTextEdit::keyPressEvent( e );
}
