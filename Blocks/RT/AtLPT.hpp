#ifndef ATLPTCOMMON_HPP
#define ATLPTCOMMON_HPP

#include "Block.hpp"

#define LPT_BASEADDR 0x378
#define EPP_DATA (LPT_BASEADDR + 4)

class AtLPT : public Block
{
	static bool input, output, isLPTOpen;
public:
	static qint32 inputOffset[ 2 ], outputOffset[ 2 ];
	enum Range
	{
		StandardRange,
		VoltageRange,
		IntegerRange,
		RangeMAX
	} static range;
	static float ioScale[ RangeMAX ];

	AtLPT( const QString &name, const QString &description, int numInputs, int numOutputs, Type type );

	bool openDevice();
	void closeDevice();
};

#endif // ATLPT_HPP
