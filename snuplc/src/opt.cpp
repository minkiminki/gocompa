#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "ir.h"
#include "ir2.h"
#include "backend2.h"
#include "opt.h"
using namespace std;


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
// Basic Block Analysis
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

  // set block info of CTacInstr
  while(it != ops.end()){
    CTacInstr_prime *instr = dynamic_cast<CTacInstr_prime*>(*it++);
    assert (instr != NULL);
    CTacLabel_prime *l = dynamic_cast<CTacLabel_prime*>(instr);
    if (l != NULL){
      blk = new CBasicBlock();
      blk->SetBlockNum(cbt->AddBlock(blk));
      l->SetFromBlock(blk);
      blk->AddInstr(instr);
    } 
    else{
      instr->SetFromBlock(blk);
      blk->AddInstr(instr);
    }
  }

  // set block contol flow info
  blk = NULL;
  it = ops.begin();
  bool cascade = false;

  while(it != ops.end()){
    CTacInstr_prime *instr = dynamic_cast<CTacInstr_prime*>(*it++);
    assert (instr != NULL);
    CBasicBlock *blk_new = instr->GetFromBlock();  
    EOperation op = instr->GetOperation();
    
    if(IsRelOp(op)) {
      CTacInstr_prime *lbl = dynamic_cast<CTacInstr_prime*>(instr->GetDest());
      assert(lbl != NULL);
      blk = lbl -> GetFromBlock();
      assert(blk != NULL);
      blk_new->AddNextBlks(blk);
      blk->AddPrevBlks(blk_new);
    }
    else if(op == opGoto) {
      CTacInstr_prime *lbl = dynamic_cast<CTacInstr_prime*>(instr->GetDest());
      assert(lbl != NULL);
      blk = lbl -> GetFromBlock();
      assert(blk != NULL);
      blk_new->AddNextBlks(blk);
      blk->AddPrevBlks(blk_new);
    }
    else if(op == opReturn) {
      cascade = false;
      cbt->AddFinBlock(instr->GetFromBlock());
    }
    else if((op == opLabel) && cascade) {
      blk_new->AddPrevBlks(blk);
      blk->AddNextBlks(blk_new);
      cascade = true;
    }
    else {
      cascade = true;
    }
    blk = blk_new;
  }    

  if(cascade){
    cbt->AddFinBlock(blk);
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

void tail_call_optimization_block(CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  list<CTacInstr*>::const_iterator it = (cbp->GetInstr()).begin();
  CTacInstr* instr0 = *it;
  if(next(it,1) != (cbp->GetInstr()).end()) {

    if(instr0->GetOperation() == opCall){
      CTacName *n = dynamic_cast<CTacName*>(instr0->GetSrc(1));
      assert(n != NULL);
      const CSymProc *proc = dynamic_cast<const CSymProc*>(n->GetSymbol());
      assert(proc != NULL);

      if(can_tail_call(proc))
	instr0->SetOperation(opTailCall);
    }    
  }

  while (it != (cbp->GetInstr()).end()) {
    CTacInstr* instr1 = *it++;
    if(instr1->GetOperation() == opReturn){
      if(it != (cbp->GetInstr()).end()){
	CTacInstr* instr2 = *it++;

	if(instr2->GetOperation() == opCall){
	  if((instr2->GetDest()) == (instr1->GetSrc(1))){
	    CTacName *n = dynamic_cast<CTacName*>(instr2->GetSrc(1));
	    assert(n != NULL);
	    const CSymProc *proc = dynamic_cast<const CSymProc*>(n->GetSymbol());
	    assert(proc != NULL);

	    if(can_tail_call(proc))
	      instr2->SetOperation(opTailCall);
	  }
	}
      }
      else{
	break;
      }
    }
  }
  return;
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
  to_ir_prime_scope(m);
  basic_block_analysis_scope(m);
  tail_call_optimization_scope(m);
  return;
}
