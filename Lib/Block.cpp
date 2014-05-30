#include "Block.hpp"
#include "Scene.hpp"
#include "Settings.hpp"

#include <QGraphicsSceneEvent>
#include <QCoreApplication>
#include <QGraphicsScene>
#include <QMimeData>
#include <QPainter>
#include <QDebug>
#include <QDrag>

#include <math.h>

static quint16 id;
int Block::sampleRate = DEFAULT_SAMPLERATE, Block::refTime = DEFAULT_REFTIME;

enum { SETTINGS = 0, DISCONNECT_INPUTS = 2, DISCONNECT_OUTPUTS = 3, DELETE = 5 };

void Block::setIDCounter( quint16 id )
{
	::id = id;
}

Block::Block( const QString &name, const QString &description, int numInputs, int numOutputs, Type type ) :
	settings( NULL ),
	label( name ),
	blocking( false ),
	name( name ), description( description ),
	type( type ),
	size( QSize( 100, 100 ) ),
	canSaveState( true ),
	pressed( false ),
	id( ++::id )
{
	setToolTip( name );

	menu.addAction( "Ustawienia" );
	menu.addSeparator();
	QAction *disconnectInputs  = menu.addAction( "Rozłącz wejścia" );
	QAction *disconnectOutputs = menu.addAction( "Rozłącz wyjścia" );
	menu.addSeparator();
	menu.addAction( "Usuń" );

	setTransformOriginPoint( size.width() / 2, size.height() / 2 );
	prostokat = QRectF( QPointF( 0.1 * size.width(), 0.1 * size.height() ), QPointF( 0.9 * size.width(), 0.9 * size.height() ) );

	setInputsCount(  type == SOURCE ? 0 : numInputs  > maxIO ? maxIO : numInputs,  false );
	setOutputsCount( type == SINK   ? 0 : numOutputs > maxIO ? maxIO : numOutputs, false );

	disconnectInputs->setEnabled( inputsCount() );
	disconnectOutputs->setEnabled( outputsCount() );
}
Block::~Block()
{
	if ( canSaveState )
		disconnectAll();
	delete settings;
}

bool Block::hasConnectedOutputs() const
{
	for ( int i = 0 ; i < outputsCount() ; ++i )
		if ( outputLines[ i ].target )
			return true;
	return false;
}
quint8 Block::calcConnectedInputs()
{
	inputsConnected = inputsDone = 0;
	for ( int i = 0 ; i < inputsCount() ; ++i )
		if ( inputConnections[ i ].second )
			++inputsConnected;
	return inputsConnected;
}

QPixmap Block::createPixmap()
{
	QPixmap pixmap( prostokat.size().toSize() + QSize( 1, 1 ) );
	pixmap.fill( Qt::white );
	QPainter p( &pixmap );
	p.translate( -prostokat.topLeft() );
	paint( &p, NULL, NULL );
	return pixmap;
}

void Block::repairConnections()
{
	if ( !targets.isEmpty() )
	{
		setOutputsCount( targets.count(), false );
		for ( int i = 0 ; i < outputsCount() ; ++i )
		{
			Block *other_block = getTargetFromID( targets[ i ].first );
			if ( other_block )
			{
				outputLines[ i ].target = other_block;
				outputLines[ i ].targetInput = targets[ i ].second;
				outputLines[ i ].target->inputConnections[ targets[ i ].second ] = qMakePair( ( quint8 )i, this );
				scene()->addItem( outputLines[ i ].line );
			}
		}
		repairLineOutputConnections();
		targets.clear();
	}
}
void Block::emitSaveState( bool removeFromScene )
{
	Scene *myScene = dynamic_cast< Scene * >( scene() );
	if ( myScene && ( removeFromScene || canSaveState ) )
	{
		if ( removeFromScene )
			myScene->removeItem( this );
		emit myScene->saveState();
	}
}

QDataStream &operator <<( QDataStream &ds, const Block *block )
{
	QVector< QPair< quint16, quint8 > > targets;
	targets.reserve( block->outputsCount() );
	for ( int i = 0 ; i < targets.capacity() ; ++i )
		targets.append( block->outputLines[ i ].target ? qMakePair( block->outputLines[ i ].target->id, block->outputLines[ i ].targetInput ) : qMakePair( quint16(), quint8() ) );
	ds << block->name.toUtf8() << block->id << block->pos() << block->inputsCount() << targets;
	block->serialize( ds );
	return ds;
}
QDataStream &operator >>( QDataStream &ds, Block *block )
{
	QPointF pos;
	quint8 inputsCount;
	ds >> block->id >> pos >> inputsCount >> block->targets;
	block->setInputsCount( inputsCount, false );
	block->deSerialize( ds );
	block->setPos( pos );
	return ds;
}

void Block::disconnectAll()
{
	canSaveState = false;
	disconnectInputs();
	disconnectOutputs();
}
void Block::disconnectInputs()
{
	bool canEmitSaveState = false;
	for ( int c = 0 ; c < inputsCount() ; ++c )
		if ( inputConnections[ c ].second )
		{
			Connection &connection = inputConnections[ c ].second->outputLines[ inputConnections[ c ].first ];
			inputConnections[ c ].second = connection.target = NULL;
			if ( scene() )
			{
				scene()->removeItem( connection.line );
				canEmitSaveState = true;
			}
		}
	if ( canEmitSaveState )
		emitSaveState();
}
void Block::disconnectOutputs()
{
	bool canEmitSaveState = false;
	for ( int c = 0 ; c < outputsCount() ; ++c )
		if ( outputLines[ c ].target )
		{
			outputLines[ c ].target = outputLines[ c ].target->inputConnections[ outputLines[ c ].targetInput ].second = NULL;
			if ( scene() )
			{
				scene()->removeItem( outputLines[ c ].line );
				canEmitSaveState = true;
			}
		}
	if ( canEmitSaveState )
		emitSaveState();
}

void Block::setInputsCount( int count, bool doUpdate )
{
	if ( inputsCount() != count )
	{
		for ( int i = inputsCount() - 1 ; i >= count ; --i )
			if ( inputConnections[ i ].second )
			{
				Connection &c = inputConnections[ i ].second->outputLines[ inputConnections[ i ].first ];
				scene()->removeItem( c.line );
				c.target = NULL;
			}
		inputs = getLinesY( count );
		inputConnections.resize( count );
		if ( doUpdate )
		{
			repairLineInputConnections();
			update();
			emitSaveState();
		}
		inputsCountChanged( count );
	}
}
void Block::setOutputsCount( int count, bool doUpdate )
{
	if ( outputsCount() != count )
	{
		for ( int i = outputsCount() - 1 ; i >= count ; --i )
			if ( outputLines[ i ].target )
				outputLines[ i ].target->inputConnections[ outputLines[ i ].targetInput ].second = NULL;
		outputs = getLinesY( count );
		outputLines.resize( count );
		if ( doUpdate )
		{
			repairLineOutputConnections();
			update();
			emitSaveState();
		}
		outputsCountChanged( count );
	}
}

void Block::repairLineOutputConnections()
{
	for ( int i = 0 ; i < outputsCount() ; ++i )
		if ( outputLines[ i ].target )
			setLine( i );
}
void Block::repairLineInputConnections()
{
	for ( int c = 0 ; c < inputsCount() ; ++c )
		if ( inputConnections[ c ].second )
			inputConnections[ c ].second->repairLineOutputConnections();
}

Block *Block::getTargetFromID( quint16 id )
{
	if ( id )
		foreach ( QGraphicsItem *item, scene()->items() )
		{
			Block *block = dynamic_cast< Block * >( item );
			if ( block && block->getID() == id )
				return block;
		}
	return NULL;
}

QVector< qreal > Block::getLinesY( int io )
{
	QVector< qreal > points;
	points.reserve( io );
	qreal step = prostokat.height() / ( io + 1.0 );
	qreal pos = prostokat.top();
	for ( int i = 0 ; i < io ; ++i )
		points.push_back( pos += step );
	return points;
}

int Block::indexOf( const QVector< qreal > &posArr, QPointF pos, bool outputs )
{
	if ( pos.x() > ( outputs ? prostokat.right() : 0.0 ) )
		for ( int i = 0 ; i < posArr.count() ; ++i )
		{
			int mid = round( posArr[ i ] );
			if ( mid == pos.y() || mid == pos.y() - 1 || mid == pos.y() + 1 )
				return i;
		}
	return -1;
}

void Block::paint( QPainter *p, const QStyleOptionGraphicsItem *, QWidget * )
{
	p->setClipRect( boundingRect() );

	QColor color;
	switch ( type )
	{
		case SOURCE:
			color = QColor( 0x60, 0xFF, 0xA0 );
			break;
		case PROCESSING:
			color = QColor( 0xFF, 0x60, 0xA0 );
			break;
		case SINK:
			color = QColor( 0x60, 0xA0, 0xFF );
			break;
	}
	p->fillRect( prostokat, pressed ? color.darker( 120 ) : color );
	p->drawRect( prostokat );

	foreach ( qreal pos, inputs )
		p->drawLine( QLineF( 0.0, pos, 0.1 * size.width(), pos ) );
	foreach ( qreal pos, outputs )
		p->drawLine( QLineF( 0.9 * size.width(), pos, size.width(), pos ) );

	p->drawText( QRectF( QPointF( prostokat.left() + 1, prostokat.top() + 1 ), QPointF( prostokat.right() - 1, prostokat.bottom() - 1 ) ), Qt::AlignCenter | Qt::TextWordWrap, label );
}

QRectF Block::boundingRect() const
{
	return QRectF( 0.0, 0.0, size.width(), size.height() );
}
void Block::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	const bool isRunning = scene()->property( "isRunning" ).toBool();
	switch ( event->button() )
	{
		case Qt::LeftButton:
			if ( !isRunning )
			{
				if ( prostokat.contains( event->pos() ) )
				{
					pressPos = event->pos();
					pressed = true;
					moved = false;
					update();
					return;
				}
				else
				{
					const int output = indexOf( outputs, event->pos(), true );
					if ( output >= 0 && !outputLines[ output ].target )
					{
						outputLines[ output ].line->setLine( QLineF( getLineStartPoint( output ), getLineStartPoint( output ) ) );
						scene()->addItem( outputLines[ output ].line );
						outputLines[ output ].underMouse = true;
						return;
					}
				}
			}
			break;
		case Qt::RightButton:
			if ( prostokat.contains( event->pos() ) && ( settings || !isRunning ) )
			{
				QList< QAction * > menuActions = menu.actions();
				for ( int i = 0 ; i <= 1 ; ++i )
					menuActions[ i ]->setVisible( settings );
				for ( int i = 2 ; i < menuActions.count() ; ++i )
					menuActions[ i ]->setVisible( !isRunning );
				switch ( menuActions.indexOf( menu.exec( event->screenPos() + QPoint( 1, 1 ) ) ) )
				{
					case SETTINGS:
						if ( settings )
						{
							settings->prepare();
							settings->show();
						}
						break;
					case DISCONNECT_INPUTS:
						disconnectInputs();
						break;
					case DISCONNECT_OUTPUTS:
						disconnectOutputs();
						break;
					case DELETE:
						disconnectAll();
						emitSaveState( true );
						delete this;
						return;
				}
			}
			break;
		default:
			break;
	}
	QGraphicsItem::mousePressEvent( event );
}
void Block::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	if ( pressed )
	{
		setPos( event->scenePos() - pressPos );
		repairLineOutputConnections();
		repairLineInputConnections();
		moved = true;
	}
	else for ( int i = 0 ; i < outputsCount() ; ++i )
	{
		if ( outputLines[ i ].underMouse )
		{
			outputLines[ i ].line->setLine( QLineF( getLineStartPoint( i ), event->scenePos() ) );
			break;
		}
	}
	QGraphicsItem::mouseMoveEvent( event );
}
void Block::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->button() == Qt::LeftButton )
	{
		if ( pressed )
		{
			pressed = false;
			if ( moved )
			{
				moved = false;
				emitSaveState();
			}
		}
		else for ( int i = 0 ; i < outputsCount() ; ++i )
			if ( outputLines[ i ].underMouse )
			{
				foreach ( QGraphicsItem *item, scene()->items( event->scenePos() ) )
				{
					Block *other_block = dynamic_cast< Block * >( item );
					if ( other_block && this != other_block )
					{
						const int input = indexOf( other_block->inputs, mapToItem( other_block, event->pos() ), false );
						if ( input > -1 && !other_block->inputConnections[ input ].second )
						{
							other_block->inputConnections[ input ] = qMakePair( ( quint8 )i, this );
							outputLines[ i ].target = other_block;
							outputLines[ i ].targetInput = input;
							outputLines[ i ].underMouse = false;
							setLine( i );
							emitSaveState();
						}
						break;
					}
				}
				if ( outputLines[ i ].underMouse )
				{
					scene()->removeItem( outputLines[ i ].line );
					outputLines[ i ].underMouse = false;
				}
				break;
			}
		update();
	}
	QGraphicsItem::mouseReleaseEvent( event );
}
