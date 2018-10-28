#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt_pre.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// give number for paramaters
void param_numbering_block(CCodeBlock *cb) {
  list<CTacInstr*>::const_iterator it = (cb->GetInstr()).end();
  int number = 0;
  while (it != (cb->GetInstr()).begin()) {
    CTacInstr* instr = *(--it);
    assert(instr != NULL);
    switch(instr->GetOperation()){
    case opCall :
      number = 0;
      break;
    case opParam :
      CTacConst *c = dynamic_cast<CTacConst*>(instr->GetDest());
      assert(c != NULL);
      c->SetValue(++number);
      break;
    }
  }
}

void param_numbering_scope(CScope *m){
  param_numbering_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    param_numbering_scope(*sit++);
  }
  return;
}


// ********************************************************************** /
// ********************************************************************** /
// IR to IR'
CCodeBlock* to_ir_prime_block(CCodeBlock *cb) {
  return (new CCodeBlock_prime(cb));
}

void to_ir_prime_scope(CScope *m) {
  m->SetCodeBlock(to_ir_prime_block(m->GetCodeBlock()));

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    to_ir_prime_scope(*sit++);
  }
  return;
}


// ********************************************************************** /
// ********************************************************************** /
// clean up
void clean_up_scope(CScope *m) {
  m->GetCodeBlock()->CleanupControlFlow();

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    clean_up_scope(*sit++);
  }
  return;
}
