#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "ui_MainWindow.h"
#include "Thread.hpp"
#include "Scene.hpp"

#include <QSettings>
#include <QStack>

class Scene;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow( QWidget *parent = NULL );
	~MainWindow();
private slots:
	void threadStopped();

	void saveState();

	void on_actionNowy_triggered();
	void on_actionOtw_rz_triggered();
	void on_actionZapisz_triggered();
	void on_actionZapisz_jako_triggered();
	void on_actionCofnij_triggered();
	void on_actionPrzywr_c_triggered();
	void on_actionStart_triggered( bool checked );
	void on_action_Ustawienia_triggered();
	void on_action_O_programie_triggered();

	void on_blocksFilterE_textChanged( const QString &txt );
private:
	void nowy();
	bool askToSave();
	void saveToFile();
	void undoRedoEnable();
	void prepareUndoRedo();
	void setItemsEnabled( bool e );

	void loadFile( const QString &fName );

	bool errorReadingScheme();

	void closeEvent( QCloseEvent *event );

	QByteArray save();
	bool restore( const QByteArray &save );

	Ui::MainWindow ui;

	Scene scene;
	Thread thread;
	QString projectFile;
	QSettings settings;
	float simulationTime;
	QList< Block * > allBlocks;
	QStack< QByteArray > undoSteps, redoSteps;
};

#endif // MAINWINDOW_HPP
