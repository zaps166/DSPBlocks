#include "MainWindow.hpp"
#include "SimSettings.hpp"
#include "Block.hpp"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QLibrary>
#include <QDebug>
#include <QFile>
#include <QDir>

#ifndef Q_OS_WIN
	#define UNIX_CDUP "/.."
	#ifndef Q_OS_MAC
		#define LIBS_FILTER "lib*.so"
	#else
		#define LIBS_FILTER "lib*.dylib"
	#endif
#else
	#define UNIX_CDUP
	#define LIBS_FILTER "*.dll"
#endif

static bool blockIDLessThan( const Block *s1, const Block *s2 )
{
	return s1->getID() < s2->getID();
}

MainWindow::MainWindow( QWidget *parent ) :
	QMainWindow( parent ),
	settings( QSettings::IniFormat, QSettings::UserScope, "MusicBlocks" ),
	simulationTime( 0.0f )
{
	setAttribute( Qt::WA_DeleteOnClose );
	ui.setupUi( this );

	qApp->setProperty( "MainWindow", QVariant::fromValue( ( void * )this ) );
	qApp->setProperty( "share", qApp->applicationDirPath() + UNIX_CDUP"/share/MusicBlocks" );

	ui.actionNowy->setIcon( QIcon::fromTheme( "document-new" ) );
	ui.actionOtw_rz->setIcon( QIcon::fromTheme( "project-open" ) );
	ui.actionZapisz->setIcon( QIcon::fromTheme( "document-save" ) );
	ui.actionZapisz_jako->setIcon( QIcon::fromTheme( "document-save-as" ) );
	ui.actionZamknij->setIcon( QIcon::fromTheme( "window-close" ) );
	ui.actionCofnij->setIcon( QIcon::fromTheme( "edit-undo" ) );
	ui.actionPrzywr_c->setIcon( QIcon::fromTheme( "edit-redo" ) );
	ui.actionStart->setIcon( QIcon::fromTheme( "media-playback-start" ) );
	ui.action_Ustawienia->setIcon( QIcon::fromTheme( "configure" ) );

	ui.graphicsView->setScene( &scene );

	const QString blocksPath = qApp->property( "share" ).toString() + "/blocks";
	foreach ( QString fName, QDir( blocksPath ).entryList( QStringList() << LIBS_FILTER ) )
	{
		QLibrary lib( blocksPath + '/' + fName );
		if ( !lib.load() )
			qDebug() << lib.errorString();
		else
		{
			typedef QList< Block * >( *CreateBlocks )();
			CreateBlocks createBlocks = ( CreateBlocks )lib.resolve( "createBlocks" );
			const char *groupName = ( const char * )lib.resolve( "groupName" );
			if ( !createBlocks )
				lib.unload();
			else foreach ( Block *block, createBlocks() )
				ui.blocksW->addBlock( block, groupName );
		}
	}
	for ( int i = 0 ; i < 3 ; ++i )
	{
		QTreeWidgetItem *tWI = ui.blocksW->topLevelItem( i );
		tWI->sortChildren( 1, Qt::AscendingOrder );
		ui.blocksW->expandItem( tWI );
	}
	for ( int i = ui.blocksW->topLevelItemCount() - 1 ; i >= 3 ; --i )
		ui.blocksW->topLevelItem( i )->sortChildren( 1, Qt::AscendingOrder );

	ui.splitter->setSizes( QList< int >() << 140 << width() - 140 );

	connect( ui.actionO_Qt, SIGNAL( triggered() ), qApp, SLOT( aboutQt() ) );
	connect( &thread, SIGNAL( finished() ), this, SLOT( threadStopped() ) );
	connect( &scene, SIGNAL( saveState() ), this, SLOT( saveState() ) );

	prepareUndoRedo();

	ui.graphicsView->setFocus();
	restoreGeometry( settings.value( "GUI/MainWinGeo" ).toByteArray() );
	ui.splitter->restoreState( settings.value( "GUI/SplitterState" ).toByteArray() );
	show();

	loadFile( settings.value( "ProjectFile" ).toString() );
}
MainWindow::~MainWindow()
{
	disconnect( &thread, SIGNAL( finished() ), this, SLOT( threadStopped() ) );

	settings.setValue( "ProjectFile", projectFile );
	settings.setValue( "GUI/MainWinGeo", saveGeometry() );
	settings.setValue( "GUI/SplitterState", ui.splitter->saveState() );

	thread.stop();
	threadStopped();
}

void MainWindow::threadStopped()
{
	foreach ( QGraphicsItem *item, scene.items() )
	{
		Block *block = dynamic_cast< Block * >( item );
		if ( block )
			block->stop();
	}
	if ( ui.actionStart->isChecked() )
		ui.actionStart->trigger();
	qApp->setProperty( "allBlocks", QVariant() );
	allBlocks.clear();
}

void MainWindow::saveState()
{
	QByteArray state = save();
	if ( undoSteps.top() != state )
	{
		undoSteps.push( state );
		redoSteps.clear();
		if ( !thread.isRunning() )
			undoRedoEnable();
		ui.actionZapisz->setEnabled( true );
	}
}

void MainWindow::on_actionNowy_triggered()
{
	if ( askToSave() )
		nowy();
}
void MainWindow::on_actionOtw_rz_triggered()
{
	if ( askToSave() )
	{
		QString fName = QFileDialog::getOpenFileName( this, "Wybierz plik schematu", projectFile, "Schematy (*.mblcks)" );
		if ( !fName.isEmpty() )
			loadFile( fName );
	}
}
void MainWindow::on_actionZapisz_triggered()
{
	if ( projectFile.isEmpty() )
		on_actionZapisz_jako_triggered();
	else
		saveToFile();
}
void MainWindow::on_actionZapisz_jako_triggered()
{
	QString fName = QFileDialog::getSaveFileName( this, "Wybierz plik schematu", QFileInfo( projectFile ).path(), "Schematy (*.mblcks)" );
	if ( !fName.isEmpty() )
	{
		projectFile = fName;
		saveToFile();
	}
}
void MainWindow::on_actionCofnij_triggered()
{
	if ( undoSteps.count() > 1 )
	{
		QByteArray state = undoSteps.pop();
		restore( undoSteps.top() );
		redoSteps.push( state );
		undoRedoEnable();
	}
}
void MainWindow::on_actionPrzywr_c_triggered()
{
	if ( !redoSteps.isEmpty() )
	{
		QByteArray state = redoSteps.pop();
		restore( state );
		undoSteps.push( state );
		undoRedoEnable();
	}
}
void MainWindow::on_actionStart_triggered( bool checked )
{
	if ( !checked )
	{
		thread.stop();
		setItemsEnabled( true );
	}
	else
	{
		bool isBlocking = false;
		QVector< Block * > sources;
		QString errorBlocks;
		foreach ( QGraphicsItem *item, scene.items() )
		{
			Block *block = dynamic_cast< Block * >( item );
			if ( block )
			{
				const bool hasConnectedOutputs = block->hasConnectedOutputs();
				bool err = false;
				if ( block->getType() == Block::SOURCE )
				{
					if ( !hasConnectedOutputs )
						continue;
					else
					{
						sources.append( block );
						allBlocks.append( block );
						isBlocking |= block->isBlocking();
					}
				}
				else if ( !block->calcConnectedInputs() && hasConnectedOutputs )
					err = true;
				else
				{
					allBlocks.append( block );
					isBlocking |= block->isBlocking();
				}
				if ( err || !block->start() )
					errorBlocks += block->getName() + '\n';
			}
		}
		if ( sources.isEmpty() || !errorBlocks.isEmpty() )
		{
			threadStopped();
			ui.actionStart->setChecked( false );
			errorBlocks.chop( 1 );
			QMessageBox::critical( this, "Błąd", errorBlocks.isEmpty() ? "Brak podłączonych źródeł" : "Poniższe bloczki zwróciły błąd:\n" + errorBlocks );
		}
		else if ( !sources.isEmpty() )
		{
			setItemsEnabled( false );
			qApp->setProperty( "allBlocks", QVariant::fromValue( ( uintptr_t )&allBlocks ) );
			thread.start( sources, ceil( simulationTime * Block::getSampleRate() ), isBlocking );
		}
	}
}
void MainWindow::on_action_Ustawienia_triggered()
{
	SimSettings simSettings( this, simulationTime, Block::getSampleRate(), Block::getRefTime() );
	if ( simSettings.exec() == QDialog::Accepted )
	{
		Block::setSampleRateAndRefTime( simSettings.getSampleRate(), simSettings.getRefTime() );
		simulationTime = simSettings.getSimulationTime();
		saveState();
	}
}
void MainWindow::on_action_O_programie_triggered()
{
	QMessageBox::information( this, QString( ( ( QAction * )sender() )->text() ).remove( '&' ), "Programista: Błażej Szczygieł" );
}

void MainWindow::on_blocksFilterE_textChanged( const QString &txt )
{
	foreach ( QTreeWidgetItem *item, ui.blocksW->findItems( QString(), Qt::MatchContains | Qt::MatchRecursive ) )
		if ( item->parent() )
			item->setHidden( !item->text( 1 ).contains( txt, Qt::CaseInsensitive ) );
}

void MainWindow::nowy()
{
	scene.clear();
	projectFile.clear();
	simulationTime = 0.0f;
	Block::setIDCounter( 0 );
	Block::resetSampleRateAndRefTime();
	prepareUndoRedo();
	ui.actionZapisz->setEnabled( false );
}
bool MainWindow::askToSave()
{
	saveState();
	if ( ui.actionZapisz->isEnabled() )
		switch ( QMessageBox::question( this, "Zapisanie zmian", "Czy chcesz zapisać zmiany?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel ) )
		{
			case QMessageBox::Yes:
				on_actionZapisz_triggered();
				break;
			case QMessageBox::Cancel:
				return false;
		}
	return true;
}
void MainWindow::saveToFile()
{
	QFile f( projectFile );
	if ( f.open( QFile::WriteOnly ) )
		f.write( save() );
	if ( f.error() )
		QMessageBox::critical( this, "Błąd zapisu", f.errorString() );
	else
		ui.actionZapisz->setEnabled( false );
}
void MainWindow::undoRedoEnable()
{
	ui.actionCofnij->setEnabled( undoSteps.count() > 1 );
	ui.actionPrzywr_c->setEnabled( !redoSteps.isEmpty() );
}
void MainWindow::prepareUndoRedo()
{
	undoSteps.clear();
	redoSteps.clear();
	undoSteps.push( save() );
	undoRedoEnable();
}
void MainWindow::setItemsEnabled( bool e )
{
	ui.actionNowy->setEnabled( e );
	ui.actionOtw_rz->setEnabled( e );
	scene.setProperty( "isRunning", !e );
	if ( e )
		undoRedoEnable();
	else
	{
		ui.actionCofnij->setEnabled( false );
		ui.actionPrzywr_c->setEnabled( false );
	}
	ui.action_Ustawienia->setEnabled( e );
}

void MainWindow::loadFile( const QString &fName )
{
	if ( fName.isEmpty() )
		return;
	QFile f( fName );
	if ( f.open( QFile::ReadOnly ) && restore( f.readAll() ) )
	{
		projectFile = fName;
		prepareUndoRedo();
		ui.actionZapisz->setEnabled( false );
	}
}

bool MainWindow::errorReadingScheme()
{
	QMessageBox::warning( this, "Wczytywanie schematu", "Błąd wczytywania zapisu schematu" );
	return false;
}

void MainWindow::closeEvent( QCloseEvent *event )
{
	if ( !askToSave() )
		event->ignore();
}

QByteArray MainWindow::save()
{
	QByteArray data;
	QDataStream ds( &data, QIODevice::WriteOnly );
	ds << simulationTime << Block::getSampleRate() << Block::getRefTime();

	QList< Block * > blocks;
	foreach ( QGraphicsItem *item, scene.items() )
	{
		Block *block = dynamic_cast< Block * >( item );
		if ( block )
			blocks << block;
	}
	qSort( blocks.begin(), blocks.end(), blockIDLessThan );

	foreach ( Block *block, blocks )
		ds << block;

	return qCompress( data );
}
bool MainWindow::restore( const QByteArray &save )
{
	QDataStream ds( qUncompress( save ) );
	if ( !save.isEmpty() && ds.atEnd() )
		return errorReadingScheme();
	QList< Block * > blocks;
	quint16 maxID = 0;
	int srate, refTime;
	ds >> simulationTime >> srate >> refTime;
	Block::setSampleRateAndRefTime( srate, refTime );
	while ( !ds.atEnd() )
	{
		Block *block = NULL;
		QByteArray name;
		ds >> name;
		foreach ( QTreeWidgetItem *item, ui.blocksW->findItems( QString(), Qt::MatchContains | Qt::MatchRecursive ) )
			if ( !name.isEmpty() && item->text( 1 ) == name )
			{
				block = ( ( Block * )item->data( 0, Qt::UserRole ).value< uintptr_t >() )->createInstance();
				blocks.append( block );
				ds >> block;
				if ( block->getID() > maxID )
					maxID = block->getID();
				break;
			}
		if ( !block )
		{
			while ( !blocks.isEmpty() )
				delete blocks.takeFirst();
			nowy();
			return errorReadingScheme();
		}
	}
	scene.clear();
	Block::setIDCounter( maxID );
	foreach ( Block *block, blocks )
		scene.addItem( block );
	foreach ( Block *block, blocks )
		block->repairConnections();
	return true;
}
