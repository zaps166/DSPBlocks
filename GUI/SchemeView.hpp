#ifndef SCHEMEVIEW_HPP
#define SCHEMEVIEW_HPP

#include <QGraphicsView>

class SchemeView : public QGraphicsView
{
public:
	inline SchemeView( QWidget *parent = NULL ) :
		QGraphicsView( parent )
	{}

	void dragEnterEvent( QDragEnterEvent *event );
private:
	void wheelEvent( QWheelEvent *event );
};

#endif // SCHEMEVIEW_HPP
