#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "ir.h"
#include "opt.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// give number for paramaters
void param_numbering_block(CCodeBlock *cb) {
  list<CTacInstr*>::const_iterator it = (cb->GetInstr()).begin();
  int number = 0;
  while (it != (cb->GetInstr()).end()) {
    CTacInstr* instr = *it++;
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
// Every Optimization
void full_optimize(CScope *m) {
  param_numbering_scope(m);
  // clean_up_scope(m);
  // to_ir_prime_scope(m);
  // basic_block_analysis_scope(m);
  // tail_call_optimization_scope(m);
  return;
}
