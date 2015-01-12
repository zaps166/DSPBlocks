#ifndef SYNTAXHIGHLIGHTER_HPP
#define SYNTAXHIGHLIGHTER_HPP

#include <QSyntaxHighlighter>

class SyntaxHighlighter : public QSyntaxHighlighter
{
public:
	typedef QList< QPair< QStringList, QTextCharFormat > > HighlightHints;

	static QTextCharFormat makeTxtChrFmt( const QBrush &color );

	inline SyntaxHighlighter( QTextDocument *document, const HighlightHints &toHighlight, const QString &comment ) :
		QSyntaxHighlighter( document ),
		toHighlight( toHighlight ),
		comment( comment )
	{}
private:
	void highlightBlock( const QString &text );

	HighlightHints toHighlight;
	QString comment;
};

#endif // LUAHIGHLIGHTER_HPP
