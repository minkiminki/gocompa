#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt_pre.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// inling DOFS
void dofs_inlining_block(CCodeBlock *cb) {
  list<CTacInstr*>::const_reverse_iterator it = (cb->GetInstr()).rbegin();
  while (it != (cb->GetInstr()).rend()) {
    CTacInstr* instr = *(it++);
    assert(instr != NULL);
    if(instr->GetOperation() == opCall){
      CTacName *n = dynamic_cast<CTacName*>(instr->GetSrc(1));
      assert(n != NULL);
      if((n->GetSymbol()->GetName()).compare("DOFS") == 0){
	while (it != (cb->GetInstr()).rend()) {
	  CTacInstr* instr2 = *it++;
	  if(instr2->GetOperation() == opParam){
	    CTacAddr *src = instr2->GetSrc(1);
	    instr->SetSrc(0, src);
	    instr2->SetOperation(opNop);
	    instr2->SetSrc(0, NULL);
	    instr2->SetDest(NULL);
	    instr->SetOperation(opDOFS);
	    break;
	  }
	}
      }
      else if((n->GetSymbol()->GetName()).compare("DIM") == 0){
	bool finished = false;
	while (it != (cb->GetInstr()).rend()) {
	  CTacInstr* instr2 = *it++;
	  if(instr2->GetOperation() == opParam){
	    CTacAddr *src0 = instr2->GetSrc(1);
	    while (it != (cb->GetInstr()).rend()) {
	      CTacInstr* instr3 = *it++;
	      if(instr3->GetOperation() == opParam){
		CTacAddr *src1 = instr3->GetSrc(1);
		instr->SetSrc(0, src0);
		instr->SetSrc(1, src1);
		instr2->SetOperation(opNop);
		instr2->SetSrc(0, NULL);
		instr2->SetDest(NULL);
		instr3->SetOperation(opNop);
		instr3->SetSrc(0, NULL);
		instr3->SetDest(NULL);
		instr->SetOperation(opDIM);
		finished = true;
		break;
	      }
	    }
	    if(finished) break;
	  }
	}
      }
    }
  }
}

void dofs_inlining_scope(CScope *m){
  dofs_inlining_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    dofs_inlining_scope(*sit++);
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


// ********************************************************************** /
// ********************************************************************** /
// fix wrong type
void pointer_typing_block(CCodeBlock *cb) {
  const CNullType* nulltyp = CTypeManager::Get()->GetNull();
  const CType* ptr_null = CTypeManager::Get()->GetPointer(nulltyp);

  list<CTacInstr*>::const_iterator it = (cb->GetInstr()).begin();
  while (it != (cb->GetInstr()).end()) {
    CTacInstr* instr = *(it++);
    assert(instr != NULL);
    if(instr->GetOperation() == opAdd){
      if(dynamic_cast<CTacReference*>(instr->GetSrc(1)) != NULL) continue;
      CTacName *c_src = dynamic_cast<CTacName*>(instr->GetSrc(1));
      if(c_src == NULL) continue;
      if(!(c_src->GetSymbol()->GetDataType()->IsPointer())) continue;
      if(dynamic_cast<CTacReference*>(c_src) != NULL) continue;
      CTacName *c_dst = dynamic_cast<CTacName*>(instr->GetDest());
      assert(c_dst != NULL);
      CSymbol* symb = const_cast<CSymbol*>(c_dst->GetSymbol());
      assert(symb != NULL);
      symb->SetDataType(ptr_null);
      // symb->SetDataType(c_src->GetSymbol()->GetDataType());
    }
    else if(instr->GetOperation() == opAssign){
      if(dynamic_cast<CTacReference*>(instr->GetSrc(1)) != NULL) continue;
      CTacName *c_src = dynamic_cast<CTacName*>(instr->GetSrc(1));
      if(c_src == NULL) continue;
      if(!(c_src->GetSymbol()->GetDataType()->IsPointer())) continue;
      if(dynamic_cast<CTacReference*>(c_src) != NULL) continue;
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
void param_numbering_function(CScope* owner, list<CTacInstr*> &instrs, list<CTacInstr*>::iterator &it, const CSymProc *proc){
  int max = proc->GetNParams();
  // list<CTacInstr*>::iterator it_next = it;
  list<CTacTemp*> params;
  for(int i = max - 1; i >= 0; i--){
    params.push_front(owner->CreateTemp(proc->GetParam(i)->GetDataType()));
  }

  {
    int i = 1;
    list<CTacTemp*>::iterator pit = params.begin();
    while(pit != params.end()){
      instrs.insert(it, new CTacInstr(opParam, new CTacConst(i++), *pit++));
      --it;
    }
  }


  list<CTacTemp*>::iterator pit = params.begin();
  while(max > 0){
    assert (it != instrs.begin());
    CTacInstr* instr = *(--it);
    assert(instr != NULL);
    switch(instr->GetOperation()){
    case opCall :
      {
	CTacName *n = dynamic_cast<CTacName*>(instr->GetSrc(1));
	assert(n != NULL);
	const CSymProc *proc = dynamic_cast<const CSymProc*>(n->GetSymbol());
	assert(proc != NULL);
	param_numbering_function(owner, instrs, it, proc);
	break;
      }
    case opParam :
      {
	instr->SetOperation(opAssign);
	instr->SetDest(new CTacName((*pit++)->GetSymbol()));
	max --; break;
      }
    }
  }
}

void param_numbering_block(CScope* owner, CCodeBlock *cb) {
  list<CTacInstr*> &instrs = const_cast<list<CTacInstr*>&>(cb->GetInstr());
  list<CTacInstr*>::iterator it = instrs.end();
  while (it != instrs.begin()) {
    CTacInstr* instr = *(--it);
    assert(instr != NULL);
    if(instr->GetOperation() == opCall){
      CTacName *n = dynamic_cast<CTacName*>(instr->GetSrc(1));
      assert(n != NULL);
      const CSymProc *proc = dynamic_cast<const CSymProc*>(n->GetSymbol());
      assert(proc != NULL);
      param_numbering_function(owner, instrs, it, proc);
    }
  }

  // list<CTacInstr*>::const_iterator it = (cb->GetInstr()).end();
  // int number = 0;
  // while (it != (cb->GetInstr()).begin()) {
  //   CTacInstr* instr = *(--it);
  //   assert(instr != NULL);
  //   switch(instr->GetOperation()){
  //   case opCall :
  //     number = 0;
  //     break;
  //   case opParam :
  //     CTacConst *c = dynamic_cast<CTacConst*>(instr->GetDest());
  //     assert(c != NULL);
  //     c->SetValue(++number);
  //     break;
  //   }
  // }

}

void param_numbering_scope(CScope *m){
  param_numbering_block(m, m->GetCodeBlock());

  vector<CScope*>::const_iterator sit = m->GetSubscopes().begin();
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
