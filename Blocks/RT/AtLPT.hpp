#ifndef ATLPTCOMMON_HPP
#define ATLPTCOMMON_HPP

#include "Block.hpp"

#define LPT_BASEADDR 0x378
#define EPP_DATA (LPT_BASEADDR + 4)

class AtLPT : public Block
{
	static bool input, output, isLPTOpen;
public:
	AtLPT( const QString &name, const QString &description, int numInputs, int numOutputs, Type type );

	bool openDevice();
	void closeDevice();
private:
	bool isOpen;
};

#endif // ATLPT_HPP
