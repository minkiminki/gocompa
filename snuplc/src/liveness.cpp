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

const CSymbol* Liveness::CreateDeadCalleeSave(int index)
{
  CSymbol* s;
  switch(index){
  case 0:
    s = new CSymbol("dead0_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 1:
    s = new CSymbol("dead1_", stRegister, CTypeManager::Get()->GetNull()); break;
  default:
    assert(false);
  }
  _tempregs.push_back(s);
  return s;
}

const CSymbol* Liveness::CreateDeadParam(int index)
{
  CSymbol* s;
  switch(index){
  case 0:
    s = new CSymbol("dead_p0_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 1:
    s = new CSymbol("dead_p1_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 2:
    s = new CSymbol("dead_p2_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 3:
    s = new CSymbol("dead_p3_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 4:
    s = new CSymbol("dead_p4_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 5:
    s = new CSymbol("dead_p5_", stRegister, CTypeManager::Get()->GetNull()); break;
  default:
    assert(false);
  }
  _tempregs.push_back(s);
  return s;
}

const CSymbol* Liveness::CreateArgReg(int index)
{
  CSymbol* s;
  switch(index){
  case 0:
    s = new CSymbol("arg0_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 1:
    s = new CSymbol("arg1_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 2:
    s = new CSymbol("arg2_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 3:
    s = new CSymbol("arg3_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 4:
    s = new CSymbol("arg4_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 5:
    s = new CSymbol("arg5_", stRegister, CTypeManager::Get()->GetNull()); break;
  default:
    assert(false);
  }
  _tempregs.push_back(s);
  return s;
}

const CSymbol* Liveness::CreateParamReg(int index)
{
  CSymbol* s;
  switch(index){
  case 0:
    s = new CSymbol("param0_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 1:
    s = new CSymbol("param1_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 2:
    s = new CSymbol("param2_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 3:
    s = new CSymbol("param3_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 4:
    s = new CSymbol("param4_", stRegister, CTypeManager::Get()->GetNull()); break;
  case 5:
    s = new CSymbol("param5_", stRegister, CTypeManager::Get()->GetNull()); break;
  default:
    assert(false);
  }
  _tempregs.push_back(s);
  return s;
}

// // list<CSymbol*>& Liveness::GetDeadRegs(void);
// // {
// //   return _deadregs;
// // }

list<CSymbol*>& Liveness::GetTempRegs(void)
{
  return _tempregs;
}
