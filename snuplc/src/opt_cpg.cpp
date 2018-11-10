#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include <cassert>

#include "opt_bba.h"
using namespace std;


// int CBasicBlock::ComputeConstants(map<CSymbol*, int*>& constants)
// {
//   if(tempinfo > 0) return 0;

//   list<CTacInstr*>::iterator it = _instrs.begin();
//   while (it != _instrs.end()) {
//     CTacInstr* instr = *it++;
//     assert(instr != NULL);

//     CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
//     if(dest == NULL) continue;
//     if(dynamic_cast<CTacReference*>(dst) != NULL) continue;

//     CTacName* s_dest = dest->GetSymbol();
//     if(s_dest == NULL) continue;
//     if(s_dest->GetSymbol(Type) != stLocal) continue;

//     CTacName* s_dest = dest->GetSymbol();
//     if(s_dest == NULL) continue;
//     CTacName* s_dest = dest->GetSymbol();
//     if(s_dest == NULL) continue;


//     switch(inst->GetOpertaion()){
//     case opAdd:





//     }


//     CTacInstr_prime *instrp = dynamic_cast<CTacInstr_prime*>(instr);
//     assert(instrp != NULL);
//     instrp->SetFromBlock(NULL);
//     // assert(cb->RemoveInstr(instr) >=0);
//   }


//   list<CBasicBlock*>::const_iterator it = _nextblks.begin();
//   while (it != _nextblks.end()){
//     CBasicBlock* blk = *it++;
//     if(blk==NULL) continue;

//     list<CBasicBlock*>::iterator fit = find(_doms.begin(), _doms.end(), blk);
//     if(fit == _doms.end()){
//       nodup_insert(_domfrontier, blk);
//     }
//   }

//   it = _doms.begin();
//   while (it != _doms.end()){
//     CBasicBlock* blk = *it++;
//     assert(blk!=NULL);

//     list<CBasicBlock*> df = blk->ComputeDF();

//     list<CBasicBlock*>::iterator it2 = df.begin();
//     while(it2 != df.end()){
//       CBasicBlock* blk2 = *it2++;
//       assert(blk2 != NULL);

//       list<CBasicBlock*>::iterator fit = find(_doms.begin(), _doms.end(), blk2);
//       if(fit == _doms.end()){
// 	nodup_insert(_domfrontier, blk2);
//       }
//     }
//   }

//   tempinfo = 1;
//   return _domfrontier;
// }


// ********************************************************************** /
// ********************************************************************** /
// Constant Propagation
int constant_propagation_block(CCodeBlock *cb) {
  int success = 0;
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  map<CSymbol*, int*> constants;
  set<CBasicBlock*> computed;
  list<CBasicBlock*> worklist;

  return success;


  // CBasicBlock* GetInitBlock(void) const initblock;



}

void constant_propagation_scope(CScope *m) {
  while(constant_propagation_block(m->GetCodeBlock()) > 0){
  }

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    constant_propagation_scope(*sit++);
  }
  return;
}
