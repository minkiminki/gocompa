#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt_pre.h"
using namespace std;



// ********************************************************************** /
// ********************************************************************** /
// fix wrong type
void pointer_typing_block(CCodeBlock *cb) {
  list<CTacInstr*>::const_iterator it = (cb->GetInstr()).begin();
  while (it != (cb->GetInstr()).end()) {
    CTacInstr* instr = *(it++);
    assert(instr != NULL);
    if(instr->GetOperation() == opAdd){
      CTacName *c_src = dynamic_cast<CTacName*>(instr->GetSrc(1));
      assert(c_src != NULL);
      if(!(c_src->GetSymbol()->GetDataType()->IsPointer())){
	if(!(c_src->GetSymbol()->GetDataType()->IsArray())) continue;
      }
      CTacName *c_dst = dynamic_cast<CTacName*>(instr->GetDest());
      assert(c_dst != NULL);
      CSymbol* symb = const_cast<CSymbol*>(c_dst->GetSymbol());
      assert(symb != NULL);
      symb->SetDataType(c_src->GetSymbol()->GetDataType());
    }
  }
}

void pointer_typing_scope(CScope *m){
  pointer_typing_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    pointer_typing_scope(*sit++);
  }
  return;
}

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
