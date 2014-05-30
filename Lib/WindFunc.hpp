#ifndef WINDFUNC_HPP
#define WINDFUNC_HPP

#include <QFrame>

class QComboBox;
class QLineEdit;

class WindFunc : public QFrame
{
	Q_OBJECT
public:
	enum WindType { Rectangle, Welch, Hamming, Hanning, Bartlett, Triangle, Bartlett_Hanning, Blackman, Nuttall, Blackman_Harris, Blackman_Nuttall, FlatTop };
	static inline QString getWindowFunction( WindType t )
	{
		return windFunc[ t ];
	}
	static QVector< double > windowFunctionCoefficient( const QString &code, int N, QString *err = NULL );

	WindFunc( QWidget *parent = NULL );

	inline QString getUserWindFunc() const
	{
		return userWindFunc;
	}
	inline void setUserWindFunc( const QString &userWindFunc )
	{
		this->userWindFunc = userWindFunc;
	}

	int getWindTypeIdx() const;
	void setWindTypeIdx( int idx );

	QVector< double > getWindowFunctionCoefficient( int N );
private slots:
	void windFuncChanged( int idx );
	void setUserWindFunc();
signals:
	void windFuncChanged();
private:
	static const QStringList windFunc;

	QComboBox *windTypeCB;
	QLineEdit *windFuncE;

	QString userWindFunc;
};

#endif // WINDFUNC_HPP
