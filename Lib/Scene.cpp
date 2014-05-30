#include "Scene.hpp"
#include "Block.hpp"

#include <QGraphicsSceneEvent>
#include <QMimeData>
#include <QDebug>

//void Scene::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
//{
//	QGraphicsScene::mouseMoveEvent( event );
//	if ( event->isAccepted() )
//		foreach ( QGraphicsItem *item, items() )
//		{
//			Block *block = dynamic_cast< Block * >( item );
//			if ( block )
//				block->repairLineOutputConnections();
//		}
//}
void Scene::dragMoveEvent( QGraphicsSceneDragDropEvent *event )
{
	if ( property( "isRunning" ).toBool() || !event->source() || !event->mimeData() || event->mimeData()->data( "Block" ).count() != sizeof( Block * ) )
		event->ignore();
}
void Scene::dropEvent( QGraphicsSceneDragDropEvent *event )
{
	if ( !property( "isRunning" ).toBool() && event->mimeData() )
	{
		QByteArray data = event->mimeData()->data( "Block" );
		if ( data.size() == sizeof( Block * ) )
		{
			Block *block = ( *( Block ** )data.data() )->createInstance();
			block->setPos( event->scenePos() );
			addItem( block );
			emit saveState();
		}
	}
}
