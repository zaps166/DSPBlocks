#include "MainWindow.hpp"
#include "SimSettings.hpp"
#ifdef Q_OS_LINUX
	#include "RTSettings.hpp"
#endif
#include "Global.hpp"
#include "Block.hpp"

#ifdef Q_OS_LINUX
	#include <QStatusBar>
#endif
#include <QCloseEvent>
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QDebug>
#include <QFile>
#include <QDir>

#include <signal.h>

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

MainWindow::MainWindow( QSettings &settings, QWidget *parent ) :
	QMainWindow( parent ),
	settings( settings ),
	simulationTime( 0.0f )
{
	setAttribute( Qt::WA_DeleteOnClose );
	ui.setupUi( this );

#ifdef Q_OS_LINUX
	ui.toolBar->addAction( actionRT = ui.menu_Symulacja->addAction( QIcon::fromTheme( "clock" ), "&Tryb czasu rzeczywistego", this, SLOT( realTimeModeSettings() ), QKeySequence( "Alt+R" ) ) );

	showRealSampleRateAction = ui.menu_Widok->addAction( "&Pokazuj rzeczywistą częstotliwość próbkowania w trybie czasu rzeczywistego", this, SLOT( showRealSampleRate() ) );
	showRealSampleRateAction->setCheckable( true );
	showRealSampleRateAction->setChecked( settings.value( "MainWindow/ShowRealSampleRate", true ).toBool() );

	setStatusBar( new QStatusBar );
	statusL = new QLabel;
	statusBar()->addWidget( statusL );
	statusBar()->hide();
	connect( &statusTimRef, SIGNAL( timeout() ), this, SLOT( updateSRate() ) );
	lastRealSampleRate = -1;
#endif

	qApp->setProperty( "share", qApp->applicationDirPath() + UNIX_CDUP"/share/DSPBlocks" );

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

	/* Ładowanie bloczków */
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

	/* Ładowanie dodatków */
	QList< QAction * > actions;
	const QString additionsPath = qApp->property( "share" ).toString() + "/additions";
	foreach ( QString fName, QDir( additionsPath ).entryList( QStringList() << LIBS_FILTER ) )
	{
		QLibrary lib( additionsPath + '/' + fName );
		if ( !lib.load() )
			qDebug() << lib.errorString();
		else
		{
			typedef QList< QAction * >( *GetActions )( QSettings & );
			GetActions getActions = ( GetActions )lib.resolve( "getActions" );
			if ( !getActions )
				lib.unload();
			else foreach ( QAction *act, getActions( settings ) )
				if ( act )
				{
					if ( !act->parentWidget() )
						act->setParent( ui.menu_Dodatki );
					else
					{
						act->parentWidget()->setParent( this );
						act->parentWidget()->setWindowFlags( Qt::Window );
					}
					actions += act;
				}
		}
	}
	if ( actions.isEmpty() )
		ui.menu_Dodatki->deleteLater();
	else
	{
		qStableSort( actions );
		int key_num = 0;
		foreach ( QAction *act, actions )
		{
			if ( key_num < 9 )
				act->setShortcut( QKeySequence( QString( "Ctrl+%1" ).arg( ++key_num ) ) );
			ui.menu_Dodatki->addAction( act );
		}
	}

	connect( ui.actionO_Qt, SIGNAL( triggered() ), qApp, SLOT( aboutQt() ) );
	connect( &thread, SIGNAL( finished() ), this, SLOT( threadStopped() ) );
	connect( &scene, SIGNAL( saveState() ), this, SLOT( saveState() ) );

	if ( ui.action_U_yj_natywnych_okien_dialogowych->isChecked() != settings.value( "NativeFileDialog", Global::isNativeFileDialog() ).toBool() )
		ui.action_U_yj_natywnych_okien_dialogowych->trigger();

#ifdef Q_OS_LINUX
	Global::setRealTime
	(
		settings.value( "RealTime/Enabled", Global::isRealTime() ).toBool(),
		settings.value( "RealTime/CPU", Global::getCPU() ).toInt(),
		settings.value( "RealTime/Sched", Global::getSched() ).toInt(),
		settings.value( "RealTime/Priority", Global::getPriority() ).toInt(),
		settings.value( "RealTime/RtMode", Global::getRtMode() ).toInt()
	);
	connect( &thread, SIGNAL( errorMessage( const QString & ) ), this, SLOT( errorMessage( const QString & ) ) );
#endif

	prepareUndoRedo();

	ui.graphicsView->setFocus();
	restoreGeometry( settings.value( "MainWindow/MainWinGeo" ).toByteArray() );
	ui.splitter->restoreState( settings.value( "MainWindow/SplitterState" ).toByteArray() );
	show();

	loadFile( settings.value( "MainWindow/ProjectFile" ).toString() );
}
MainWindow::~MainWindow()
{
	disconnect( &thread, SIGNAL( finished() ), this, SLOT( threadStopped() ) );

	settings.setValue( "MainWindow/ProjectFile", projectFile );
	settings.setValue( "MainWindow/MainWinGeo", saveGeometry() );
	settings.setValue( "MainWindow/SplitterState", ui.splitter->saveState() );

#ifdef Q_OS_LINUX
	settings.setValue( "MainWindow/ShowRealSampleRate", showRealSampleRateAction->isChecked() );

	settings.setValue( "RealTime/Enabled", Global::isRealTime() );
	settings.setValue( "RealTime/Sched", Global::getSched() );
	settings.setValue( "RealTime/Priority", Global::getPriority() );
	settings.setValue( "RealTime/CPU", Global::getCPU() );
	settings.setValue( "RealTime/RtMode", Global::getRtMode() );
#endif

	if( thread.stop() )
		threadStopped();
	else
	{
		qDebug().nospace() << "Wymuszenie zakończenia " << qApp->applicationName() << "...";
		::raise( SIGTERM );
	}

	qApp->quit();
}

void MainWindow::threadStopped()
{
#ifdef Q_OS_LINUX
	stopStatusUpdates();
#endif
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
		QString fName = QFileDialog::getOpenFileName( this, "Wybierz plik schematu", projectFile, "Schematy (*.dblcks)", NULL, Global::getNativeFileDialogFlag() );
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
	QString fName = QFileDialog::getSaveFileName( this, "Wybierz plik schematu", projectFile.isEmpty() ? "Schemat.dblcks" : QFileInfo( projectFile ).path(), "Schematy (*.dblcks)", NULL, Global::getNativeFileDialogFlag() );
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
		if ( thread.stop() )
			setItemsEnabled( true );
		else
		{
			ui.actionStart->setChecked( true );
			QMessageBox::warning( this, qApp->applicationName(), "Program nie odpowiada. Zapisz projekt i zakończ pracę programu." );
		}
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
				{
					QString blockErr = block->getError();
					errorBlocks += "<li><b>" + block->getName() + "</b>" + ( blockErr.isEmpty() ? QString() : " - " + blockErr ) + "</li>";
				}
			}
		}
		if ( sources.isEmpty() || !errorBlocks.isEmpty() )
		{
			threadStopped();
			ui.actionStart->setChecked( false );
			errorBlocks.chop( 1 );
			QMessageBox::critical( this, "Błąd", errorBlocks.isEmpty() ? "Brak podłączonych źródeł" : "Poniższe bloczki zwróciły błąd:<ul>" + errorBlocks + "</ul>" );
		}
		else if ( !sources.isEmpty() )
		{
			setItemsEnabled( false );
			qApp->setProperty( "allBlocks", QVariant::fromValue( ( quintptr )&allBlocks ) );
#ifdef Q_OS_LINUX
			if ( Global::isRealTime() )
			{
				if ( isBlocking )
					QMessageBox::warning( this, "Tryb czasu rzeczywistego", "Używane są bloczki blokujące, tryb czasu rzeczywistego nie zostanie aktywowany." );
				else
					startStatusUpdates();
			}
#endif
			thread.start( sources, ceil( simulationTime * Global::getSampleRate() ), isBlocking );
		}
	}
}
void MainWindow::on_action_Ustawienia_triggered()
{
	SimSettings simSettings( this, simulationTime );
	if ( simSettings.exec() == QDialog::Accepted )
	{
		Global::setSampleRateAndRefTime( simSettings.getSampleRate(), simSettings.getRefTime() );
		simulationTime = simSettings.getSimulationTime();
		saveState();
	}
}
void MainWindow::on_action_U_yj_natywnych_okien_dialogowych_triggered( bool n )
{
	Global::setNativeFileDialog( n );
	settings.setValue( "NativeFileDialog", n );
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

void MainWindow::errorMessage( const QString &msg )
{
#ifdef Q_OS_LINUX
	QMessageBox::critical( this, "Tryb czasu rzeczywistego", msg );
#endif
}
void MainWindow::realTimeModeSettings()
{
#ifdef Q_OS_LINUX
	RTSettings rtSettings( this );
	rtSettings.exec();
#endif
}
void MainWindow::showRealSampleRate()
{
#ifdef Q_OS_LINUX
	if ( thread.isRealTimeNow() )
	{
		stopStatusUpdates();
		startStatusUpdates();
	}
#endif
}
void MainWindow::updateSRate()
{
#ifdef Q_OS_LINUX
	qint32 realSampleRate = thread.getRealSampleRate();
	if ( realSampleRate != lastRealSampleRate )
	{
		if ( realSampleRate >= 0 )
			statusL->setText( QString( "Próbkowanie: %1 Hz" ).arg( realSampleRate / 10.0, 12, 'f', 1 ) );
		lastRealSampleRate = realSampleRate;
	}
#endif
}

void MainWindow::nowy()
{
	scene.clear();
	projectFile.clear();
	simulationTime = 0.0f;
	Block::setIDCounter( 0 );
	Global::resetSampleRateAndRefTime();
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
#ifdef Q_OS_LINUX
	actionRT->setEnabled( e );
#endif
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
	ds << simulationTime << Global::getSampleRate() << Global::getRefTime();

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
	Global::setSampleRateAndRefTime( srate, refTime );
	while ( !ds.atEnd() )
	{
		Block *block = NULL;
		QByteArray name;
		ds >> name;
		foreach ( QTreeWidgetItem *item, ui.blocksW->findItems( QString(), Qt::MatchContains | Qt::MatchRecursive ) )
			if ( !name.isEmpty() && item->text( 1 ) == name )
			{
				block = ( ( Block * )item->data( 0, Qt::UserRole ).value< quintptr >() )->createInstance();
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

void MainWindow::startStatusUpdates()
{
#ifdef Q_OS_LINUX
	if ( showRealSampleRateAction->isChecked() )
	{
		statusBar()->show();
		statusTimRef.start( 750 );
	}
#endif
}
void MainWindow::stopStatusUpdates()
{
#ifdef Q_OS_LINUX
	if ( statusTimRef.isActive() )
	{
		statusTimRef.stop();
		statusBar()->hide();
		lastRealSampleRate = 0;
		statusL->clear();
	}
#endif
}
