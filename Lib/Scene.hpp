#ifndef SCENE_HPP
#define SCENE_HPP

#include <QGraphicsScene>

class Scene : public QGraphicsScene
{
	Q_OBJECT
public:
//	void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
	void dragMoveEvent( QGraphicsSceneDragDropEvent *event );
	void dropEvent( QGraphicsSceneDragDropEvent *event );

	Q_SIGNAL void saveState();
};

#endif // SCENE_HPP
