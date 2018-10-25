#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt.h"
using namespace std;

#define ADMIT (assert(false));

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
// Basic Block Analysis

bool is_final_instr(EOperation e){
  return ((e == opReturn) || (e == opGoto));
}

void basic_block_analysis_block(CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  CBlockTable *cbt = cbp->GetBlockTable();
  assert(cbt != NULL);

  list<CTacInstr*> ops = cbp->GetInstr();

  list<CTacInstr*>::const_iterator it = ops.begin();

  CBasicBlock *blk = new CBasicBlock();
  blk->SetBlockNum(cbt->AddBlock(blk));
  cbt->SetInitBlock(blk);
  blk->AddPrevBlks(NULL);
  bool cascade = true;
  bool is_first = false;

  // set block info of CTacInstr
  while(it != ops.end()){
    CTacInstr_prime *instr = dynamic_cast<CTacInstr_prime*>(*it++);
    assert (instr != NULL);
    EOperation o = instr->GetOperation();
    
    if (o == opLabel){
      CBasicBlock* blk_new = new CBasicBlock();
      blk_new->SetBlockNum(cbt->AddBlock(blk_new));
      if(cascade){
	blk_new->AddPrevBlks(blk);
	blk->AddNextBlks(blk_new);
      }
      blk = blk_new;
      instr->SetFromBlock(blk);
      blk->AddInstr(instr);
      cascade = true;
      is_first = false;
    }
    else if(cascade){
      if(is_first){
	CBasicBlock* blk_new = new CBasicBlock();
	blk_new->SetBlockNum(cbt->AddBlock(blk_new));
	blk_new->AddPrevBlks(blk);
	blk->AddNextBlks(blk_new);
	blk = blk_new;
      }
      assert(blk != NULL);
      instr->SetFromBlock(blk);
      blk->AddInstr(instr);
      cascade = !is_final_instr(o);
      is_first = IsRelOp(o);
    }
  }
  if(cascade){
    blk->AddNextBlks(NULL);
    cbt->AddFinBlock(blk);
  }

  // set block contol flow info
  it = ops.begin();
  while(it != ops.end()){
    CTacInstr_prime *instr = dynamic_cast<CTacInstr_prime*>(*it++);
    assert (instr != NULL);
    blk = instr->GetFromBlock();

    if(blk == NULL){
      continue;
    }
    else if(instr->IsBranch()) {
      CTacInstr_prime *lbl = dynamic_cast<CTacInstr_prime*>(instr->GetDest());
      assert(lbl != NULL);
      CBasicBlock *blk_new = lbl->GetFromBlock();
      assert(blk_new != NULL);

      blk->AddNextBlks(blk_new);
      blk_new->AddPrevBlks(blk);
    }
    else if((instr->GetOperation()) == opReturn) {
      blk->AddNextBlks(NULL);
      cbt->AddFinBlock(blk);
    }

  }

  // eliminate unused basic block
  bool success = true;
  while(success){
    success = false;
    list<CBasicBlock*>::const_iterator it = (cbt->GetBlockList()).begin();
    while (it != (cbt->GetBlockList()).end()) {
      CBasicBlock *blk = *it++;
      assert(blk != NULL);
      if((blk->GetPrevBlks()).begin() == (blk->GetPrevBlks()).end()){
  	cbt->RemoveBlock(blk);
  	success = true; break;
      }
    }    
  }

  // combine block
  success = true;
  while(success){
    success = false;
    list<CBasicBlock*>::const_iterator it = (cbt->GetBlockList()).begin();
    while (it != (cbt->GetBlockList()).end()) {
      CBasicBlock *blk = *it++;
      assert(blk != NULL);
      if(next((blk->GetNextBlks()).begin(), 1) == (blk->GetNextBlks()).end()){
  	CBasicBlock *blk_next = *(blk->GetNextBlks()).begin();
  	if(blk_next != NULL){
  	  if(next((blk_next->GetPrevBlks()).begin(), 1) == (blk_next->GetPrevBlks()).end()){
  	    cbt->CombineBlock(blk, blk_next);
  	    success = true; break;
  	  }
  	}
      }
    }    
  }

}

void basic_block_analysis_scope(CScope *m) {
  basic_block_analysis_block(m->GetCodeBlock());
  
  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    basic_block_analysis_scope(*sit++);
  }
  return;
}


// ********************************************************************** /
// ********************************************************************** /
// Tail Call optimization
bool can_tail_call(const CSymProc* proc){
  int n = proc->GetNParams();
  if(n>6) return false;

  for(int i=1; i<=n; i++){
    const CSymParam* param = proc->GetParam(i);
    const CType *t = param->GetDataType();
    if((t->IsPointer()) || (t->IsArray())){
      return false;
    }
  }
  return true;
}

// bool can_tail_call(const CSymProc* proc){
//   int n = proc->GetNParams();
//   if(n>6) return false;

//   for(int i=1; i<=n; i++){
//     const CSymParam* param = proc->GetParam(i);
//     const CType *t = param->GetDataType();
//     if((t->IsPointer()) || (t->IsArray())){
//       return false;
//     }
//   }
//   return true;
// }

void tail_call_optimization_block(CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  CBlockTable *cbt = cbp->GetBlockTable();
  assert(cbt != NULL);

  CTacName *n;

  list<CBasicBlock*>::const_iterator it = (cbt->GetFinBlocks()).begin();
  while (it != (cbt->GetFinBlocks()).end()){
    list<CTacInstr*> instrs = (*it++)->GetInstrs();
    list<CTacInstr*>::const_reverse_iterator iit = instrs.rbegin();

    while((iit != instrs.rend())){
      assert(*iit != NULL);
      if((*iit)->GetOperation() == opNop){
	iit++;
	continue;
      }
      else{
	break;
      }
    }

    CTacInstr* instr1;
    CTacInstr* instr0;

    if(iit != instrs.rend()){
      instr0 = *iit++;
      if(instr0->GetOperation() == opReturn){

	while(iit != instrs.rend()){
	  assert(*iit != NULL);
	  if((*iit)->GetOperation() == opNop){
	    iit++;
	    continue;
	  }
	  else{
	    break;
	  }
	}

	if(iit != instrs.rend()){
	   instr1 = *iit++;
	   if(instr1->GetOperation() == opCall){

	     if((instr1->GetDest()) == (instr0->GetSrc(1))){
	     }
	     else{
	       continue;
	     }
	   }
	   else{
	     continue;
	   }
	 }
	 else{
	   continue;
	 }	 
      }
      else if(instr0->GetOperation() == opCall){
	instr1 = instr0;
      }
      else{
	continue;
      }
    }
    else{
      continue;
    }
    
    CTacName *n = dynamic_cast<CTacName*>(instr1->GetSrc(1));
    assert(n != NULL);
    const CSymProc *proc = dynamic_cast<const CSymProc*>(n->GetSymbol());
    assert(proc != NULL);
    
    int i = proc->GetNParams();
    if(i>6) continue;

    bool exitloop = false;
    for(; i >= 1; i--){
      if(iit == instrs.rend()){
	exitloop = true;
	break;
      }
      instr0 = *iit++;
      assert(instr0 != NULL);

      if(instr0->GetOperation() == opParam){
        n = dynamic_cast<CTacName*>(instr0->GetSrc(1));
	assert(n != NULL);
	const CSymbol *s = n->GetSymbol();
	assert(s != NULL);
        if((s->GetDataType()->IsPointer()) || (s->GetDataType()->IsArray())){
	  if((s->GetSymbolType() != stGlobal) && (s->GetSymbolType() != stParam)){

	    if(iit != instrs.rend()){
	      instr0 = *iit;
	      assert(instr0 != NULL);
	      if(instr0->GetOperation() == opAddress){
		if(n == dynamic_cast<CTacName*>(instr0->GetDest())){
		  n = dynamic_cast<CTacName*>(instr0->GetSrc(1));
		  assert(n != NULL);
		  const CSymbol *s = n->GetSymbol();
		  assert(s != NULL);
		  if((s->GetSymbolType() == stGlobal) || (s->GetSymbolType() == stParam)){
		    i--;
		    continue;
		  }
		}
	      }
	    }
	    exitloop = true;
	    break;
	  }
	}
	i--;
      }
    }

    if(exitloop){
      break;
    }
    else{
      instr1->SetOperation(opTailCall);
    }
  }
}

void tail_call_optimization_scope(CScope *m){
  tail_call_optimization_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    tail_call_optimization_scope(*sit++);
  }
  return;
}


// ********************************************************************** /
// ********************************************************************** /
// Every Optimization
void full_optimize(CScope *m) {
  param_numbering_scope(m);
  clean_up_scope(m);
  to_ir_prime_scope(m);
  basic_block_analysis_scope(m);
  tail_call_optimization_scope(m);
  return;
}
