#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt_bba.h"
using namespace std;


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
