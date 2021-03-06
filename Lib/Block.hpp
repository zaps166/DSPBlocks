#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <QCoreApplication>
#include <QGraphicsItem>
#include <QMenu>
#include <QPen>

template< typename T > class Array;
class Settings;

class Block : public QGraphicsItem
{
	Q_DISABLE_COPY( Block )
	friend class Settings;
	friend class Scene;
public:
	enum Type { SOURCE, PROCESSING, SINK };

	struct Sample
	{
		QPair< Block *, quint8 > target;
		float sample;
	};

	static void setIDCounter( quint16 id );

	Block( const QString &name, const QString &description, int numInputs, int numOutputs, Type type );
	virtual ~Block();

	inline QString getName() const
	{
		return name;
	}
	inline QString getDescription() const
	{
		return description;
	}
	inline Type getType() const
	{
		return type;
	}
	inline quint16 getID() const
	{
		return id;
	}
	inline bool isBlocking() const
	{
		return blocking;
	}

	bool hasConnectedOutputs() const;
	quint8 calcConnectedInputs();
	inline bool allConnectedInputsHasSample()
	{
		if ( inputsConnected == ++inputsDone )
		{
			inputsDone = 0;
			return true;
		}
		return false;
	}

	virtual bool start() = 0;
	virtual void setSample( int input, float sample ) { Q_UNUSED( input ) Q_UNUSED( sample ) }
	virtual void exec( Array< Sample > &samples ) { Q_UNUSED( samples ) }
	virtual void stop() = 0;

	inline QString getError()
	{
		QString tmp = err;
		err.clear();
		return tmp;
	}

	QPixmap createPixmap();

	inline quint8 inputsCount() const
	{
		return inputs.count();
	}
	inline quint8 outputsCount() const
	{
		return outputs.count();
	}

	virtual Block *createInstance() = 0;

	void repairConnections();
	void emitSaveState( bool removeFromScene = false );

	friend QDataStream &operator <<( QDataStream &ds, const Block *block );
	friend QDataStream &operator >>( QDataStream &ds, Block *block );
protected:
	static const quint8 maxIO = 10;

	inline QPair< Block *, quint8 > getTarget( int output ) const
	{
		return qMakePair( outputLines[ output ].target, outputLines[ output ].targetInput );
	}

	template< typename T > QList< T * > getBlocksByType() const
	{
		QList< Block * > *allBlocks = ( QList< Block * > * )qApp->property( "allBlocks" ).value< quintptr >();
		QList< T * > blocksByType;
		if ( allBlocks )
			foreach ( Block *block, *allBlocks )
			{
				T *b = dynamic_cast< T * >( block );
				if ( b )
					blocksByType.append( b );
			}
		return blocksByType;
	}

	virtual void serialize( QDataStream &ds ) const { Q_UNUSED( ds ) }
	virtual void deSerialize( QDataStream &ds ) { Q_UNUSED( ds ) }

	virtual void inputsCountChanged( int num ) { Q_UNUSED( num ) }
	virtual void outputsCountChanged( int num ) { Q_UNUSED( num ) }

	Settings *settings;
	QString label, err;
	bool blocking;
private:
	void disconnectAll();
	void disconnectInputs();
	void disconnectOutputs();

	void setInputsCount( int count, bool doUpdate = true );
	void setOutputsCount( int count, bool doUpdate = true );
	void setSize();

	void repairLineOutputConnections();
	void repairLineInputConnections();

	Block *getTargetFromID( quint16 id );

	QVector< qreal > getLinesY( int io );

	inline QPointF getLineStartPoint( int output )
	{
		return mapToScene( size.width(), outputs[ output ] );
	}
	inline void setLine( const int output )
	{
		Connection &c = outputLines[ output ];
		c.line->setLine( QLineF( getLineStartPoint( output ), mapToScene( mapFromItem( c.target, 0, c.target->inputs[ c.targetInput ] ) ) ) );
	}

	int indexOf( const QVector< qreal > &posArr, QPointF pos, bool outputs );

	void setGradient( const QColor &color );

	void paint( QPainter *p, const QStyleOptionGraphicsItem *, QWidget * );

	QRectF boundingRect() const;
	void mousePressEvent( QGraphicsSceneMouseEvent *event );
	void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );

	QLinearGradient normalGrad, pressedGrad;
	QString name, description;
	QRectF prostokat;
	Type type;
	QSize size;

	class Connection
	{
		quint32 *ref;
	public:
		inline Connection() :
			ref( new quint32( 1 ) ),
			line( new QGraphicsLineItem ),
			target( NULL ),
			underMouse( false )
		{
			line->setPen( QColor( 0xFF2277EE ) );
		}
		inline Connection( const Connection &other )
		{
			*this = other;
			++*ref;
		}
		inline ~Connection()
		{
			if ( !--*ref )
			{
				delete line;
				delete ref;
			}
		}

		QGraphicsLineItem *line;
		Block *target;
		quint8 targetInput;
		bool underMouse;
	};

	quint8 inputsConnected, inputsDone;
	QVector< QPair< quint16, quint8 > > targets; /* Do zapisu połączeń (target_id, target_input_number) */

	QVector< QPair< quint8, Block * > > inputConnections; /* Połączenia wejść (other_block_output_number, other_block) */
	QVector< Connection > outputLines; /* Połączenia wyjść + dodatkowe informacje GUI */
	bool canSaveState, moved, pressed;
	QVector< qreal > inputs, outputs;
	QPointF pressPos;
	quint16 id;
	QMenu menu;
};

#endif // BLOCK_HPP
