extern const char groupName[] = "Logic";

#include "Logic.hpp"

extern "C" QList< Block * > createBlocks()
{
	return QList< Block * >()
		<< new Logic( Logic::VALUE, "Logic value", "Generuje stan wysoki lub niski" )
		<< new Logic( Logic::BUFFER, "Buffer", "Przepuszcza sygnał cyfrowy" )
		<< new Logic( Logic::NOT, "NOT", "Neguje sygnał cyfrowy" )
		<< new Logic( Logic::AND, "AND", "Bramka AND" )
		<< new Logic( Logic::NAND, "NAND", "Bramka NAND" )
		<< new Logic( Logic::OR, "OR", "Bramka OR" )
		<< new Logic( Logic::NOR, "NOR", "Bramka NOR" )
		<< new Logic( Logic::XOR, "XOR", "Bramka XOR" )
		<< new Logic( Logic::XNOR, "XNOR", "Bramka XNOR" );
}
