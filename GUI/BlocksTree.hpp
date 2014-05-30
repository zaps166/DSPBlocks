#ifndef BLOCKSTREE_HPP
#define BLOCKSTREE_HPP

#include <QTreeWidget>

class Block;

class BlocksTree : public QTreeWidget
{
public:
	BlocksTree( QWidget *parent = NULL );

	void addBlock( Block *block, const char *groupName );
private:
	void mouseMoveEvent( QMouseEvent *event );
};

#endif // BLOCKSTREE_HPP
