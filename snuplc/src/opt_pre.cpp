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


// ********************************************************************** /
// ********************************************************************** /
// reference division
void div_ref_block(CScope *m, CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);


  const CType* inttyp = CTypeManager::Get()->GetInt();
  list<CBasicBlock*>::const_iterator bit = cbp->GetBlockTable()->GetBlockList().begin();
  while (bit != cbp->GetBlockTable()->GetBlockList().end()) {
    CBasicBlock* blk = *bit++;
    list<CTacInstr*> & instrs = const_cast<list<CTacInstr*> &>(blk->GetInstrs());

    list<CTacInstr*>::iterator it = instrs.begin();


    while (it != instrs.end()) {
      list<CTacInstr*>::iterator it_temp = it++;
      CTacInstr* instr = *it_temp;
      assert(instr != NULL);

      if(instr->GetOperation() == opDiv){

	CTacAddr *c_src = instr->GetSrc(2);
	if(dynamic_cast<CTacName*>(c_src) == NULL){
	  if(dynamic_cast<CTacConst*>(c_src) == NULL) continue;
	}
	else{
	  if(dynamic_cast<CTacReference*>(c_src) == NULL) continue;
	}
	CTacTemp* _s_new = m->CreateTemp(inttyp);
	CTacInstr_prime* instr_new = new CTacInstr_prime(opAssign, _s_new, c_src,
						       NULL);
	instr_new->SetFromBlock(blk);
	instr->SetSrc(1, _s_new);

	{
	  list<CTacInstr*> & instrs_global = const_cast<list<CTacInstr*> &>(cb->GetInstr());
	  list<CTacInstr*>::iterator fit = find(instrs_global.begin(), instrs_global.end(), instr);
	  assert(fit != instrs_global.end());
	  instrs_global.insert(fit, instr_new);
	}

	instrs.insert(it_temp, instr_new);
      }
      else {

      }

    }
  }


}

void div_ref_scope(CScope *m){
  div_ref_block(m, m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    div_ref_scope(*sit++);
  }
  return;
}
