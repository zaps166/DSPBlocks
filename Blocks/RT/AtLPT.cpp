#include "AtLPT.hpp"

#include <sys/io.h>

bool AtLPT::input = false, AtLPT::output = false, AtLPT::isLPTOpen = false;

AtLPT::AtLPT( const QString &name, const QString &description, int numInputs, int numOutputs, Block::Type type ) :
	Block( name, description, numInputs, numOutputs, type ),
	isOpen( false )
{}

bool AtLPT::openDevice()
{
	if ( ( getType() == SOURCE && input ) || ( getType() == SINK && output ) )
	{
		err = "Dopuszczalny jest tylko jeden bloczek wejściowy i jeden bloczek wyjściowy dla AtLPT!";
		return false;
	}
	if ( !isLPTOpen )
	{
		if ( ioperm( LPT_BASEADDR, 5, 1 ) )
		{
			err = "Błąd przydzielania uprawnień do portu LPT1. Sprawdź, czy masz uprawnienia root'a!";
			return false;
		}
		outb( 0x20, LPT_BASEADDR + 2 );
		outb( 0x20, EPP_DATA );
		isLPTOpen = true;
	}
	switch ( getType() )
	{
		case SOURCE:
			input = true;
			break;
		case SINK:
			output = true;
			break;
		default:
			break;
	}
	isOpen = true;
	return true;
}
void AtLPT::closeDevice()
{
	if ( isOpen )
	{
		switch ( getType() )
		{
			case SOURCE:
				input = false;
				break;
			case SINK:
				output = false;
				break;
			default:
				break;
		}
		if ( !input && !output )
		{
			ioperm( LPT_BASEADDR, 5, 0 );
			isLPTOpen = false;
		}
	}
}
