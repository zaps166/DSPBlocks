#ifndef LADSPA_HPP
#define LADSPA_HPP

#include "Block.hpp"

class QLibrary;

#include "ladspa.h"

class LADSPA : public Block
{
	friend class LADSPA_UI;
public:
	LADSPA( const QString &name, int numInputs, int numOutputs, const LADSPA_Descriptor &ld );

	bool start();
	void setSample( int in_buffer, float sample );
	void exec( Array< Sample > &samples );
	void stop();

	Block *createInstance();
private:
	void serialize( QDataStream &ds ) const;
	void deSerialize( QDataStream &ds );

	int pos;
	bool multiinstances;
	const LADSPA_Descriptor &ld;
	QVector< QVector< LADSPA_Data > > buffer;
	QVector< LADSPA_Handle > ladspa_instances;
};

#include "Settings.hpp"

class LADSPA_UI : public AdditionalSettings
{
	Q_OBJECT
public:
	LADSPA_UI( LADSPA &block );

	QList< QWidget * > controlWList;
	QVector< LADSPA_Data > control;
private slots:
	void setValue( int v );
	void setValue( double v );
private:
	LADSPA &block;
};

#endif // LADSPA_HPP
