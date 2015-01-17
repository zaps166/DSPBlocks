#include "SchemeView.hpp"

#include <QDragEnterEvent>
#include <QWheelEvent>

void SchemeView::dragEnterEvent( QDragEnterEvent *event )
{
	if ( event && event->mimeData() && !event->mimeData()->hasUrls() )
		return QGraphicsView::dragEnterEvent( event );
}

void SchemeView::wheelEvent( QWheelEvent *event )
{
	if ( !( event->modifiers() & Qt::ControlModifier ) )
		QGraphicsView::wheelEvent( event );
	else
	{
		if ( event->delta() > 0 )
			scale( 1.25, 1.25 );
		else if ( event->delta() < 0 )
			scale( 0.8, 0.8 );
	}
}
