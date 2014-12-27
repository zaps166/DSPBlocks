#include "Clip.hpp"
#include "Const.hpp"
#include "Counter.hpp"
#include "Delay.hpp"
#include "Differential.hpp"
#include "FileReader.hpp"
#include "FileWriter.hpp"
#include "FIR.hpp"
#include "Gain.hpp"
#include "IIR.hpp"
#include "Integral.hpp"
#include "JS.hpp"
#include "Math.hpp"
#include "Multiplier.hpp"
#include "Muxer.hpp"
#include "PWM.hpp"
#include "Quantizer.hpp"
#include "RandomGen.hpp"
#include "Share.hpp"
#include "RMS.hpp"
#include "Scope.hpp"
#include "Sine.hpp"
#include "StdOut.hpp"
#include "Summation.hpp"
#include "VarDelay.hpp"

extern "C" QList< Block * > createBlocks()
{
	return QList< Block * >() <<
		 new Clip <<
		 new Const <<
		 new Counter <<
		 new Delay <<
		 new Differential <<
		 new FileReader <<
		 new FileWriter <<
		 new FIR <<
		 new Gain <<
		 new IIR <<
		 new Integral <<
		 new JS <<
		 new Math <<
		 new Multiplier <<
		 new Muxer <<
		 new PWM <<
		 new Quantizer <<
		 new RandomGen <<
		 new Share <<
		 new RMS <<
		 new Scope <<
		 new Sine <<
		 new StdOut <<
		 new Summation <<
		 new VarDelay;
}
