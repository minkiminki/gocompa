#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "backend.h"
using namespace std;

bool CSymbol::isInReg(void)
{
	if( _rbase.compare("%rbp") == 0) return false;
	else return true;
}
