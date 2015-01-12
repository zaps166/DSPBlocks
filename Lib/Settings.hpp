#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <QFrame>

class QPushButton;
class QSpinBox;
class QLabel;
class Block;

class AdditionalSettings : public QFrame
{
public:
	AdditionalSettings( Block &block );

	virtual void prepare() {}
	virtual void setRunMode( bool b ) { Q_UNUSED( b ) }
	virtual bool canClose();
private:
	Block &block;
};

class Settings : public QWidget
{
	Q_OBJECT
public:
	Settings( Block &block, bool canModifyInputs, quint8 inputsMin, quint8 inputsMax, bool canModifyOutputs, quint8 outputsMin, quint8 outputsMax, bool independent = false, AdditionalSettings *additionalSettings = NULL );

	template< typename T > inline T *getAdditionalSettings()
	{
		return reinterpret_cast< T * >( additionalSettings );
	}
	inline AdditionalSettings *getAdditionalSettings()
	{
		return additionalSettings;
	}

	void prepare();
	void setRunMode( bool b );
private slots:
	void inputsCountChanged( int num );
	void outputsCountChanged( int num );
private:
	void closeEvent( QCloseEvent *event );

	class IO : public QWidget
	{
	public:
		IO( const QString &labelTxt, int min, int max );

		void connectValueChanged( QObject *obj, const char *slot );

		QLabel *label;
		QSpinBox *num;
	} *inputs, *outputs;

	QPushButton *closeB;

	Block &block;
	AdditionalSettings *additionalSettings;
};

#endif // SETTINGS_HPP
