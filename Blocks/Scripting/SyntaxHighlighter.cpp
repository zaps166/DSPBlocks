#include "SyntaxHighlighter.hpp"

static inline bool chkFragment( const QString &txt )
{
	return txt.isEmpty() || txt == " " || txt == "\t" || txt == "(" || txt == ")" || txt == "{" || txt == "}" || txt == "[" || txt == "]" || txt == ":" || txt == ";" || txt == "." || txt == "#";
}

QTextCharFormat SyntaxHighlighter::makeTxtChrFmt( const QBrush &color )
{
	QTextCharFormat txtChrFmt;
	txtChrFmt.setForeground( color );
	txtChrFmt.setFontWeight( QFont::Bold );
	return txtChrFmt;
}

void SyntaxHighlighter::highlightBlock( const QString &text )
{
	QString txt = text;

	const int commentStartIdx = txt.indexOf( comment );
	if ( commentStartIdx > -1 )
	{
		const int len = txt.length() - commentStartIdx;
		setFormat( commentStartIdx, len, QColor( Qt::gray ) );
		txt.remove( commentStartIdx, len );
	}

	for ( int i = 0 ; i < toHighlight.count() ; ++i )
		foreach ( QString t, toHighlight[ i ].first )
		{
			int idx = -1;
			while ( ( idx = txt.indexOf( t, idx + 1 ) ) > -1 )
			{
				const QString before = text.mid( idx - 1, 1 );
				const QString after = text.mid( idx + t.length(), 1 );
				if ( chkFragment( before ) && chkFragment( after ) )
					setFormat( idx, t.length(), toHighlight[ i ].second );
			}
		}
}
