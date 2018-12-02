#include <iomanip>
#include <cassert>
#include <map>
#include <algorithm>
#include <list>
#include <set>

#include "symtab.h"
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
  _allocated[_param_regs[5]] = rgR9;

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
  _param_numb++;
  _tempregs.push_back(s);
  _arg_regs[index] = s;
  // _arg_regs.push_back(s);
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


map<const CSymbol*, list<const CSymbol*>>& Liveness::GetAssignClosure(void)
{
  return _assign_closure;
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

void Liveness::debug_print(void){

  cout << "================================================" << endl;
  cout << "max : " << _max << endl;
  cout << "================================================" << endl;

  {
    cout << "liveness ---------------------------------------------" << endl;
    map<const CSymbol*, list<const CSymbol*>>::iterator git = _live_graph.begin();
    while (git != _live_graph.end()) {
      if(git->first->GetSymbolType() == stRegister) {
	git++;
	continue;
      }
      cout << (git->first->GetName()) << " -";
      list<const CSymbol*> &slist = git->second;
      list<const CSymbol*>::const_iterator sit = slist.begin();
      while (sit != slist.end()) {
	cout << " " << (*sit++)->GetName();
      }
      cout << endl;
      git++;
    }
  }

  {
    cout << "assign ---------------------------------------------" << endl;
    map<const CSymbol*, list<const CSymbol*>>::iterator git = _assign_graph.begin();
    while (git != _assign_graph.end()) {
      if(git->first->GetSymbolType() == stRegister) {
  	git++;
  	continue;
      }
      cout << (git->first->GetName()) << " -";
      list<const CSymbol*> &slist = git->second;
      list<const CSymbol*>::const_iterator sit = slist.begin();
      while (sit != slist.end()) {
  	cout << " " << (*sit++)->GetName();
      }
      cout << endl;
      git++;
    }
  }

  {
    cout << "symbols ---------------------------------------------" << endl;
    list<const CSymbol*>::iterator sit = _symblist.begin();
    while (sit != _symblist.end()) {
      cout << (*sit++)->GetName() << " ";
    }
    cout << endl;
  }


  {
    cout << "allocated ---------------------------------------------" << endl;

    map<const CSymbol*, ERegister>::iterator git = _allocated.begin();
    while (git != _allocated.end()) {
      cout << (git->first->GetName()) << " - ";

      if((git->second) <= rgMAX){
	cout << ERegName[git->second];
      }
      else{
	cout << "in stack";
      }

      // switch(git->second){
      // case rgR9: cout << "R9"; break;
      // case rgR8: cout << "R8"; break;
      // case rgRCX: cout << "RCX"; break;
      // case rgRSI: cout << "RSI"; break;
      // case rgRDI: cout << "RDI"; break;
      // case rgRBX: cout << "RBX"; break;
      // case rgR12: cout << "R12"; break;
      // case rgR13: cout << "R13"; break;
      // case rgR14: cout << "R14"; break;
      // case rgR15: cout << "R15"; break;
      // case rgR10: cout << "R10"; break;
      // case rgR11: cout << "R11"; break;
      // default: cout << "in stack";
      // }

      cout << endl;
      git++;
    }
  }

}

int Liveness::Allocate(void)
{
  int max = 0;
  ERegister min = rgMIN;

  list<const CSymbol*>::const_iterator sit = _symblist.begin();
  while (sit != _symblist.end()) {
    const CSymbol* s = *sit++;

    list<const CSymbol*> & _assigns = _assign_graph[s];
    list<const CSymbol*> & _assigns_closure = _assign_closure[s];
    list<const CSymbol*> & _lives = _live_graph[s];

    list<ERegister> can;
    {
      list<const CSymbol*>::const_iterator cit = _assigns.begin();
      while (cit != _assigns.end()) {
    	const CSymbol* c = *cit++;
    	if(_allocated.find(c) != _allocated.end()){
    	  nodup_insert(can, _allocated[c]);
    	}
      }
    }

    list<ERegister> cant;
    {
      list<const CSymbol*>::const_iterator cit = _lives.begin();
      while (cit != _lives.end()) {
	const CSymbol* c = *cit++;
	if(find(_assigns.begin(), _assigns.end(), c) != _assigns.end()) continue;
	if(_allocated.find(c) != _allocated.end()){
	  nodup_insert(cant, _allocated[c]);
	}
      }
    }

    list<ERegister> may;
    {
      list<const CSymbol*>::const_iterator cit = _assigns_closure.begin();
      while (cit != _assigns_closure.end()) {
    	const CSymbol* c = *cit++;
    	if(find(_assigns.begin(), _assigns.end(), c) != _assigns.end()) continue;
    	if(find(_lives.begin(), _lives.end(), c) != _lives.end()) {
	  continue;
	}
    	if(_allocated.find(c) != _allocated.end()){
    	  nodup_insert(may, _allocated[c]);
    	}
      }
    }

    {
      bool success = false;
      list<ERegister>::const_iterator eit = can.begin();
      while (eit != can.end()) {
        ERegister e = *eit++;
	if(e > rgMAX){
	  continue;
	}
	if(find(cant.begin(), cant.end(), e) != cant.end()) continue;
	if(e > max) max = e;
	_allocated[s] = e;
	const_cast<CSymbol*>(s)->SetName(s->GetName() + "_" + ERegName[e]);
	success = true;
	break;
      }
      if(success) continue;
    }

    {
      bool success = false;
      list<ERegister>::const_iterator eit = may.begin();
      while (eit != may.end()) {
        ERegister e = *eit++;
    	if(e > rgMAX){
    	  continue;
    	}
    	if(find(cant.begin(), cant.end(), e) != cant.end()) continue;
    	if(e > max) max = e;
    	_allocated[s] = e;
    	const_cast<CSymbol*>(s)->SetName(s->GetName() + "_" + ERegName[e]);
    	success = true;
    	break;
      }
      if(success) continue;
    }

    {
      ERegister e = min;
      while(true){
	if(find(cant.begin(), cant.end(), e) != cant.end()){
	  e = (ERegister)((int)e + 1);
	  continue;
	}
	else{
	  break;
	}
      }
      if(e > max) max = e;
      _allocated[s] = e;
      // cout << ERegName[e] << endl;
      // _P2;

      // TODO : erase it
      if(e <= rgMAX){
	const_cast<CSymbol*>(s)->SetName(s->GetName() + "_" + ERegName[e]);
      }
      else{
	const_cast<CSymbol*>(s)->SetName(s->GetName() + "_stk" + to_string(e-rgMAX));
      }

    }
  }

  _max = max;
  return max;
}

int Liveness::GetMax()
{
  return _max;
}

int Liveness::GetParamNum()
{
  return _param_numb;
}

map<int, const CSymbol*> & Liveness::GetArgRegs(void)
{
  return _arg_regs;
}
