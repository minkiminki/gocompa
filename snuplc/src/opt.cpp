#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "ir.h"
#include "ir2.h"
#include "backend2.h"
#include "opt.h"
using namespace std;


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

void full_optimize(CScope *m) {
  to_ir_prime_scope(m);
  basic_block_analysis_scope(m);
  return;
}
