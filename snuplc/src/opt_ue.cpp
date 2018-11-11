#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <set>

#include "opt_ue.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Remove Unused Labels
void remove_label_block(CCodeBlock *cb) {
  set<CTacInstr*> labels;

  list<CTacInstr*>::const_iterator it = cb->GetInstr().begin();
  while (it != cb->GetInstr().end()) {
    CTacInstr* instr = *it++;
    if(dynamic_cast<CTacLabel_prime*>(instr) != NULL){
      cout << "insert " << instr << endl;
      labels.insert(instr);
    }
  }

  it = cb->GetInstr().begin();
  while (it != cb->GetInstr().end()) {
    CTacInstr* instr = *it++;
    if(instr->GetOperation() == opGoto){
      CTacLabel_prime *dest = dynamic_cast<CTacLabel_prime*>(instr->GetDest());
      assert(dest != NULL);
      if(labels.erase(dest) > 0){
	cout << "remove " << dest << endl;
      }
    }
  }

  set<CTacInstr*>::iterator sit = labels.begin();
  while (sit != labels.end()) {
    CTacInstr* instr = *sit++;
    cout << instr << endl;
    cb->RemoveInstr(instr);
    // assert(cb->RemoveInstr(instr) >= 0);
  }

}

void remove_label_scope(CScope *m) {
  remove_label_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    remove_label_scope(*sit++);
  }
  return;
}


// ********************************************************************** /
// ********************************************************************** /
// Remove Useless Goto
void remove_goto_block(CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  list<CTacInstr*>::const_iterator it = cb->GetInstr().begin();
  while (it != cb->GetInstr().end()) {
    CTacInstr* instr = *it++;
    CTacInstr* instr2 = *it;
    if(instr->IsBranch() && instr->GetDest() == instr2){
      cb->RemoveInstr(instr);
      _P2;
    }
  }
}

void remove_goto_scope(CScope *m) {
  remove_goto_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    remove_goto_scope(*sit++);
  }
  return;
}


// ********************************************************************** /
// ********************************************************************** /
// Remove Nops
void remove_nop_block(CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);
  cbp->RemoveNop();
}

void remove_nop_scope(CScope *m) {
  remove_nop_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    remove_nop_scope(*sit++);
  }
  return;
}


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

  cbt->BlockRenumber(cb->GetInstr());

  // set block info of CTacInstr
  while(it != ops.end()){
    list<CTacInstr*>::const_iterator it0 = it;
    CTacInstr_prime *instr = dynamic_cast<CTacInstr_prime*>(*it++);
    assert (instr != NULL);
    if(instr->GetFromBlock() == NULL){
      assert(cb->RemoveInstr(instr) >= 0);
    }
  }

  // cbp->RemoveNop();
}

void unused_elimination_scope(CScope *m) {
  unused_elimination_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    unused_elimination_scope(*sit++);
  }
  return;
}
