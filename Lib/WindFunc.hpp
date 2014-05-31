#ifndef WINDFUNC_HPP
#define WINDFUNC_HPP

#include <QFrame>

class QPushButton;
class QComboBox;
class QLineEdit;

class WindFunc : public QFrame
{
	Q_OBJECT
public:
	enum WindType { Rectangle, Welch, Hamming, Hanning, Bartlett, Triangle, Bartlett_Hanning, Blackman, Nuttall, Blackman_Harris, Blackman_Nuttall, FlatTop, Kaiser };
	static inline QString getWindowFunction( WindType t )
	{
		return ( t >= Rectangle && t <= FlatTop ) ? windFunc[ t ] : QString();
	}
	static QVector< double > windowFunctionCoefficients( const QString &code, int N, QString *err = NULL );

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

	void setKaiserEditEnabled( bool e );
	void setKaiserBeta( double beta );

	QVector< double > getWindowFunctionCoefficient( int N );
private slots:
	void windFuncChanged( int idx );
	void setUserWindFunc();
	void showGraph();
signals:
	void windFuncChanged( bool isKaiser );
private:
	static const QStringList windFunc;

	QComboBox *windTypeCB;
	QPushButton *showGraphB;
	QLineEdit *windFuncE;

	bool kaiserEditEnabled;
	QString userWindFunc;
	double kaiserBeta;
};

#endif // WINDFUNC_HPP
