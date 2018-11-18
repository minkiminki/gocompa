#include <iomanip>
#include <cassert>
#include <map>
#include <algorithm>
#include <list>
#include <set>

#include "ir2.h"
using namespace std;


//------------------------------------------------------------------------------
// Liveness
//

Liveness::Liveness()
{
  const CNullType* nulltyp = CTypeManager::Get()->GetNull();

  _param_regs[0] = new CSymbol("%rdi", stRegister, nulltyp);
  _param_regs[1] = new CSymbol("%rsi", stRegister, nulltyp);
  _param_regs[2] = new CSymbol("%rdx", stRegister, nulltyp);
  _param_regs[3] = new CSymbol("%rcx", stRegister, nulltyp);
  _param_regs[4] = new CSymbol("%r8", stRegister, nulltyp);
  _param_regs[5] = new CSymbol("%r9", stRegister, nulltyp);

  _caller_save1 = new CSymbol("%r10", stRegister, nulltyp);
  _caller_save2 = new CSymbol("%r11", stRegister, nulltyp);
}

const CSymbol** Liveness::GetParamRegs(void)
{
  return _param_regs;
}

const CSymbol* Liveness::GetCallerSave(int index)
{
  if(index == 1){
    return _caller_save1;
  }
  else{
    assert(index == 2);
    return _caller_save2;
  }
}

map<CBasicBlock*, list<const CSymbol*>>& Liveness::GetUses(int index)
{
  if(index == 1){
    return _uses1;
  }
  else{
    assert(index == 2);
    return _uses2;
  }
}
