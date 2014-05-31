#include "GraphW.hpp"

#include <QPainter>
#include <QDebug>

#include <math.h>

GraphW::GraphW( QWidget *parent ) :
	QWidget( parent )
{
	setStyle( &style );
}

void GraphW::set_y_samples( const QVector< double > &y_samples )
{
	path = QPainterPath();
	path.moveTo( 0.0, -y_samples[ 0 ] );
	min = max = y_samples[ 0 ];
	for ( int x = 1 ; x < y_samples.count() ; ++x )
	{
		path.lineTo( x, -y_samples[ x ] );
		if ( y_samples[ x ] < min )
			min = y_samples[ x ];
		if ( y_samples[ x ] > max )
			max = y_samples[ x ];
	}
	if ( min > 0.0f )
		min = 0.0f;
	update();
}

void GraphW::paintEvent( QPaintEvent * )
{
	if ( !path.isEmpty() )
	{
		QPainter p( this );

		p.scale( width() / ( path.elementCount() - 1.0 ), ( height() - 1.0 ) / ( max - min ) );
		p.translate( 0.0, max );

		p.setPen( QPen( Qt::darkGray, 0.0, Qt::DotLine ) );
		p.drawLine( QLineF( 0.0, 0.0, path.elementCount(), 0.0 ) );

		p.setPen( QPen( Qt::black, 0.0 ) );
		p.drawPath( path );
	}
}
