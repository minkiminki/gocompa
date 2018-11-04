#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt_ue.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// instruction renumbering
void instruction_renumber_scope(CScope *m) {
  m->GetCodeBlock()->InstrRenumber();

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    instruction_renumber_scope(*sit++);
  }
  return;
}


// ********************************************************************** /
// ********************************************************************** /
// eliminate unused instr & block
void unused_elimination_block(CCodeBlock *cb) {
  const list<CTacInstr*> ops = cb->GetInstr();
  list<CTacInstr*>::const_iterator it = ops.begin();

  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  CBlockTable *cbt = cbp->GetBlockTable();
  assert(cbt != NULL);

  cbt->BlockRenumber();

  // set block info of CTacInstr
  while(it != ops.end()){
    list<CTacInstr*>::const_iterator it0 = it;
    CTacInstr_prime *instr = dynamic_cast<CTacInstr_prime*>(*it++);
    assert (instr != NULL);
    if(instr->GetFromBlock() == NULL){
      assert(cb->RemoveInstr(instr) >= 0);
    }
  }
}

void unused_elimination_scope(CScope *m) {
  unused_elimination_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    unused_elimination_scope(*sit++);
  }
  return;
}
