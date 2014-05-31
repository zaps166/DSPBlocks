#ifndef GRAPHW_HPP
#define GRAPHW_HPP

#include <QCommonStyle>
#include <QWidget>

class GraphW : public QWidget
{
	Q_OBJECT
public:
	GraphW( QWidget *parent = NULL );

	void set_y_samples( const QVector< double > &y_samples );
private:
	void paintEvent( QPaintEvent * );

	QCommonStyle style;
	QPainterPath path;
	float min, max;
};

#endif // GRAPHW_HPP
