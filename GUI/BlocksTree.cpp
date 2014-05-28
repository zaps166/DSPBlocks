#include "BlocksTree.hpp"
#include "Block.hpp"

#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>

BlocksTree::BlocksTree( QWidget *parent ) :
	QTreeWidget( parent )
{}

void BlocksTree::addBlock( Block *block, const char *groupName )
{
	QTreeWidgetItem *parent = NULL;
	if ( !groupName )
		parent = topLevelItem( block->getType() );
	else
	{
		QList< QTreeWidgetItem * > items = findItems( groupName, Qt::MatchContains );
		if ( !items.isEmpty() )
			parent = items[ 0 ];
		else
		{
			parent = new QTreeWidgetItem( this );
			parent->setText( 0, groupName );
		}
	}
	QTreeWidgetItem *item = new QTreeWidgetItem( parent );
	item->setText( 1, block->getName() );
	item->setToolTip( 0, block->getDescription() );
	item->setData( 0, Qt::DecorationRole, block->createPixmap() );
	item->setData( 0, Qt::UserRole, QVariant::fromValue( ( uintptr_t )block ) );
}

void BlocksTree::mouseMoveEvent( QMouseEvent *event )
{
	if ( event->buttons() & Qt::LeftButton )
	{
		QTreeWidgetItem *tWI = itemAt( event->pos() );
		if ( tWI && tWI->parent() && tWI->isSelected() )
		{
			QMimeData *mimeData = new QMimeData;
			Block *block = ( Block * )tWI->data( 0, Qt::UserRole ).value< uintptr_t >();
			mimeData->setData( "Block", QByteArray( ( const char * )&block, sizeof block ) );

			QDrag *drag = new QDrag( this );
			drag->setPixmap( tWI->data( 0, Qt::DecorationRole ).value< QPixmap >() );
			drag->setMimeData( mimeData );
			drag->exec();
		}
	}
	QTreeWidget::mouseMoveEvent( event );
}
