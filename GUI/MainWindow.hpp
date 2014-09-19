#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "ui_MainWindow.h"
#include "Thread.hpp"
#include "Scene.hpp"

#include <QStack>
#include <QTimer>

class QSettings;
class Scene;
#ifdef Q_OS_LINUX
	class QLabel;
#endif

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow( QSettings &settings, QWidget *parent = NULL );
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
	void realTimeModeSettings();
	void on_action_U_yj_natywnych_okien_dialogowych_triggered( bool n );
	void showRealSampleRate();
	void on_action_O_programie_triggered();

	void on_blocksFilterE_textChanged( const QString &txt );

	void errorMessage( const QString &msg );
	void updateSRate();
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

	void startStatusUpdates();
	void stopStatusUpdates();

	Ui::MainWindow ui;

#ifdef Q_OS_LINUX
	QAction *actionRT, *showRealSampleRateAction;
	qint32 lastRealSampleRate;
	QTimer statusTimRef;
	QLabel *statusL;
#endif

	Scene scene;
	Thread thread;
	QString projectFile;
	QSettings &settings;
	float simulationTime;
	QList< Block * > allBlocks;
	QStack< QByteArray > undoSteps, redoSteps;
};

#endif // MAINWINDOW_HPP
