#ifndef SYNTAXHIGHLIGHTER_HPP
#define SYNTAXHIGHLIGHTER_HPP

#include <QSyntaxHighlighter>

class SyntaxHighlighter : public QSyntaxHighlighter
{
public:
	typedef QList< QPair< QStringList, QTextCharFormat > > HighlightHints;

	static QTextCharFormat makeTxtChrFmt( const QBrush &color );

	inline SyntaxHighlighter( QTextDocument *document, const HighlightHints &toHighlight ) :
		QSyntaxHighlighter( document ),
		toHighlight( toHighlight )
	{}
private:
	void highlightBlock( const QString &text );

	HighlightHints toHighlight;
};

#endif // LUAHIGHLIGHTER_HPP
