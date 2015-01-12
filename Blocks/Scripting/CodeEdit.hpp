#ifndef CODEEDIT_HPP
#define CODEEDIT_HPP

#include <QTextEdit>

class CodeEdit : public QTextEdit
{
	Q_OBJECT
public:
	CodeEdit();
private slots:
	void blockCountChanged( int blockCount );
	void cursorPositionChanged();
signals:
	void lineNumber( int line );
private:
	void focusOutEvent( QFocusEvent *e );
	void focusInEvent( QFocusEvent *e );
	void keyPressEvent( QKeyEvent *e );

	int lastBlockCount;
};

#endif // CODEEDIT_HPP
