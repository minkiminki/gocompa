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
  deadnum = 0;
  tempnum = 0;
  // const CNullType* nulltyp = CTypeManager::Get()->GetNull();

  _param_regs[0] = new CSymRegister("%rdi", CTypeManager::Get()->GetNull());
  _allocated[_param_regs[0]] = rgRDI;
  _param_regs[1] = new CSymRegister("%rsi", CTypeManager::Get()->GetNull());
  _allocated[_param_regs[1]] = rgRSI;
  // _param_regs[2] = new CSymRegister("%rdx", CTypeManager::Get()->GetNull());
  // _allocated[_param_regs[2]] = RDX;
  _param_regs[3] = new CSymRegister("%rcx", CTypeManager::Get()->GetNull());
  _allocated[_param_regs[3]] = rgRCX;
  _param_regs[4] = new CSymRegister("%r8", CTypeManager::Get()->GetNull());
  _allocated[_param_regs[4]] = rgR8;
  _param_regs[5] = new CSymRegister("%r9", CTypeManager::Get()->GetNull());
  _allocated[_param_regs[5]] = rgR8;

  _caller_save1 = new CSymRegister("%r10", CTypeManager::Get()->GetNull());
  _allocated[_caller_save1] = rgR10;
  _caller_save2 = new CSymRegister("%r11", CTypeManager::Get()->GetNull());
  _allocated[_caller_save2] = rgR11;
}

// const CSymbol** Liveness::GetParamRegs(void)
// {
//   return _param_regs;
// }

// const CSymbol* Liveness::GetCallerSave(int index)
// {
//   if(index == 1){
//     return _caller_save1;
//   }
//   else{
//     assert(index == 2);
//     return _caller_save2;
//   }
// }

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

const CSymbol* Liveness::GetDeadCalleeSave(int index)
{
  if(index == 1){
    return _caller_save1;
  }
  else{
    assert(index == 2);
    return _caller_save2;
  }
}

const CSymbol* Liveness::GetDeadParam(int index)
{
  return _param_regs[index];
}


// const CSymbol* Liveness::CreateDeadCalleeSave(int index)
// {
//   CSymbol* s;
//   switch(index){
//   case 0:
//     s = new CSymRegister("d"+to_string(deadnum++), CTypeManager::Get()->GetNull()); break;
//   case 1:
//     s = new CSymRegister("d"+to_string(deadnum++), CTypeManager::Get()->GetNull()); break;
//   default:
//     assert(false);
//   }
//   _tempregs.push_back(s);
//   return s;
// }

// const CSymbol* Liveness::CreateDeadParam(int index)
// {
//   CSymbol* s;
//   switch(index){
//   case 0:
//     s = new CSymRegister("d"+to_string(deadnum++), CTypeManager::Get()->GetNull()); break;
//   case 1:
//     s = new CSymRegister("d"+to_string(deadnum++), CTypeManager::Get()->GetNull()); break;
//   case 2:
//     s = new CSymRegister("d"+to_string(deadnum++), CTypeManager::Get()->GetNull()); break;
//   case 3:
//     s = new CSymRegister("d"+to_string(deadnum++), CTypeManager::Get()->GetNull()); break;
//   case 4:
//     s = new CSymRegister("d"+to_string(deadnum++), CTypeManager::Get()->GetNull()); break;
//   case 5:
//     s = new CSymRegister("d"+to_string(deadnum++), CTypeManager::Get()->GetNull()); break;
//   default:
//     assert(false);
//   }
//   _tempregs.push_back(s);
//   return s;
// }


const CSymbol* Liveness::CreateArgReg(int index)
{
  CSymbol* s;
  switch(index){
  case 0:
    s = new CSymbol("a"+to_string(tempnum++), stLocal, CTypeManager::Get()->GetNull());
    _allocated[s] = rgRDI;
    break;
  case 1:
    s = new CSymbol("a"+to_string(tempnum++), stLocal, CTypeManager::Get()->GetNull());
    _allocated[s] = rgRSI;
    break;
  case 2:
    assert(false);
    // s = new CSymbol("a"+to_string(tempnum++), stLocal, CTypeManager::Get()->GetNull());
    // _allocated[s] = RDX;
    // break;
  case 3:
    s = new CSymbol("a"+to_string(tempnum++), stLocal, CTypeManager::Get()->GetNull());
    _allocated[s] = rgRCX;
    break;
  case 4:
    s = new CSymbol("a"+to_string(tempnum++), stLocal, CTypeManager::Get()->GetNull());
    _allocated[s] = rgR8;
    break;
  case 5:
    s = new CSymbol("a"+to_string(tempnum++), stLocal, CTypeManager::Get()->GetNull());
    _allocated[s] = rgR9;
    break;
  default:
    assert(false);
  }
  _tempregs.push_back(s);
  return s;
}

// const CSymbol* Liveness::CreateArgReg(int index)
// {
//   CSymbol* s;
//   switch(index){
//   case 0:
//     s = new CSymRegister("a"+to_string(tempnum++), CTypeManager::Get()->GetNull()); break;
//   case 1:
//     s = new CSymRegister("a"+to_string(tempnum++), CTypeManager::Get()->GetNull()); break;
//   case 2:
//     s = new CSymRegister("a"+to_string(tempnum++), CTypeManager::Get()->GetNull()); break;
//   case 3:
//     s = new CSymRegister("a"+to_string(tempnum++), CTypeManager::Get()->GetNull()); break;
//   case 4:
//     s = new CSymRegister("a"+to_string(tempnum++), CTypeManager::Get()->GetNull()); break;
//   case 5:
//     s = new CSymRegister("a"+to_string(tempnum++), CTypeManager::Get()->GetNull()); break;
//   default:
//     assert(false);
//   }
//   _tempregs.push_back(s);
//   return s;
// }

const CSymbol* Liveness::CreateParamReg(int index)
{
  CSymbol* s;
  switch(index){
  case 0:
    s = new CSymRegister("p"+to_string(tempnum++), CTypeManager::Get()->GetNull());
    _allocated[s] = rgRDI;
     break;
  case 1:
    s = new CSymRegister("p"+to_string(tempnum++), CTypeManager::Get()->GetNull());
    _allocated[s] = rgRSI;
    break;
  case 2:
    assert(false);
    // s = new CSymRegister("p"+to_string(tempnum++), CTypeManager::Get()->GetNull());
    // _allocated[s] = RDX;
    // break;
  case 3:
    s = new CSymRegister("p"+to_string(tempnum++), CTypeManager::Get()->GetNull());
    _allocated[s] = rgRCX;
    break;
  case 4:
    s = new CSymRegister("p"+to_string(tempnum++), CTypeManager::Get()->GetNull());
    _allocated[s] = rgR8;
    break;
  case 5:
    s = new CSymRegister("p"+to_string(tempnum++), CTypeManager::Get()->GetNull());
    _allocated[s] = rgR9;
    break;
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

list<const CSymbol*>& Liveness::GetTempRegs(void)
{
  return _tempregs;
}

map<const CSymbol*, list<const CSymbol*>>& Liveness::GetAssignGraph(void)
{
  return _assign_graph;
}

map<const CSymbol*, list<const CSymbol*>>& Liveness::GetLiveGraph(void)
{
  return _live_graph;
}

map<const CSymbol*, ERegister> & Liveness::GetAllocated(void)
{
  return _allocated;
}

list<const CSymbol*> & Liveness::GetSymbList(void)
{
  return _symblist;
}
