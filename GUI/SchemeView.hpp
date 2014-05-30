#ifndef SCHEMEVIEW_HPP
#define SCHEMEVIEW_HPP

#include <QGraphicsView>

class SchemeView : public QGraphicsView
{
public:
	inline SchemeView( QWidget *parent = NULL ) :
		QGraphicsView( parent )
	{}
private:
	void wheelEvent( QWheelEvent *event );
//	void mouseMoveEvent();
};

#endif // SCHEMEVIEW_HPP
