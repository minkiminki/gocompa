#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <map>

#include "backend.h"
using namespace std;


void CSymbol::SetSymbolType(ESymbolType symbt)
{
  _symboltype = symbt;
}

bool CSymbol::isInReg(void) const
{
  if( _rbase.compare("") == 0) return false;

  // if( _rbase.compare("%rbp") == 0) return false;
  else return true;
}

bool CSymtab::RemoveSymbol(const string name)
{
  map<string, CSymbol*>::iterator it = _symtab.find(name);
  if(it == _symtab.end()) return false;
  else{
    _symtab.erase(it);
    return true;
  }
}
