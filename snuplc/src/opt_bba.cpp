#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt_bba.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Remove Unused Labels
void remove_unreachable_block(CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  CBlockTable *cbt = cbp->GetBlockTable();
  assert(cbt != NULL);

  // eliminate unused basic block
  bool success = true;
  while(success){
    success = false;
    list<CBasicBlock*>::const_iterator it = (cbt->GetBlockList()).begin();
    while (it != (cbt->GetBlockList()).end()) {
      CBasicBlock *blk = *it++;
      assert(blk != NULL);
      if((blk->GetPrevBlks()).begin() == (blk->GetPrevBlks()).end()){
  	cbt->RemoveBlock(cbp, blk);
  	success = true; break;
      }
    }
  }
}

// ********************************************************************** /
// ********************************************************************** /
// Basic Block Analysis
bool is_final_instr(EOperation e){
	return ((e == opReturn) || (e == opGoto));
}

bool is_join(CBasicBlock* blk){
  if(blk == NULL){
    return true;
  }
  else if(next((blk->GetPrevBlks()).begin(), 1) == (blk->GetPrevBlks()).end()){
    return false;
  }
  else{
       assert(next((blk->GetPrevBlks()).begin(), 2) == (blk->GetPrevBlks()).end());
    return true;
  }
}

bool is_fork(CBasicBlock* blk){
  if(blk == NULL){
    return false;
  }
  else if(next((blk->GetNextBlks()).begin(), 1) == (blk->GetNextBlks()).end()){
    return false;
  }
  else{
       assert(next((blk->GetNextBlks()).begin(), 2) == (blk->GetNextBlks()).end());
    return true;
  }
}

void basic_block_analysis_block(CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  cbp->AddInitialLabel();

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

  remove_unreachable_block(cb);

  // // eliminate unused basic block
  // bool success = true;
  // while(success){
  //   success = false;
  //   list<CBasicBlock*>::const_iterator it = (cbt->GetBlockList()).begin();
  //   while (it != (cbt->GetBlockList()).end()) {
  //     CBasicBlock *blk = *it++;
  //     assert(blk != NULL);
  //     if((blk->GetPrevBlks()).begin() == (blk->GetPrevBlks()).end()){
  // 	cbt->RemoveBlock(cbp, blk);
  // 	success = true; break;
  //     }
  //   }
  // }

  // TODO: remove unreachable block
  // TODO: insert empty block for critical edge

  // combine block
  bool success = true;
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

  // remove critical edge
  list<CBasicBlock*>::const_iterator bit = (cbt->GetBlockList()).begin();
  while(bit != (cbt->GetBlockList()).end()) {
    CBasicBlock *blk = *bit++;
    assert(blk != NULL);
    list<CBasicBlock*>::const_iterator bit_next = (blk->GetNextBlks()).begin();
    while(bit_next != (blk->GetNextBlks()).end()){
      CBasicBlock *blk_next = *bit_next++;
      if(blk_next != NULL){
  	if(is_fork(blk) && is_join(blk_next)){ // check critical edge
  	  CTacInstr *instr = *(blk->GetInstrs().rbegin());
  	  assert(IsRelOp(instr->GetOperation()));
  	  CTacLabel_prime* lb = dynamic_cast<CTacLabel_prime*>(instr->GetDest());
  	  assert(lb != NULL);
  	  if(lb->GetFromBlock() == blk_next){
  	    CTacInstr_prime *instrp = dynamic_cast<CTacInstr_prime*>(instr);
  	    cbp->SplitIf(instrp);
  	  }
  	  else{
  	    cbp->SplitElse(blk, blk_next);
  	  }
  	}
      }
    }
  }

  // critical edge checking
  bit = (cbt->GetBlockList()).begin();
  while(bit != (cbt->GetBlockList()).end()) {
    CBasicBlock *blk = *bit++;
    assert(blk != NULL);
    list<CBasicBlock*>::const_iterator bit_next = (blk->GetNextBlks()).begin();
    while(bit_next != (blk->GetNextBlks()).end()){
      CBasicBlock *blk_next = *bit_next++;
      if(blk_next != NULL){
  	assert(!is_fork(blk) || !is_join(blk_next));
      }
    }
  }

  // dominance relation
  list<CBasicBlock*> blks = cbt->GetBlockList();
  blks.push_back(NULL);

  list<CBasicBlock*> root_pdoms;
  root_pdoms.push_back(NULL);

  bit = (cbt->GetBlockList()).begin();
  while(bit != (cbt->GetBlockList()).end()) {
    CBasicBlock *blk = *bit++;
    blk->SetPreDoms(blks);
  }

  success = true;
  while (success){
    success = false;
    bit = (cbt->GetBlockList()).begin();
    while(bit != (cbt->GetBlockList()).end()) {
      CBasicBlock *blk = *bit++;
      assert(blk != NULL);

      list<CBasicBlock*>::const_iterator bit2 = blk->GetPrevBlks().begin();
      assert(bit2 != blk->GetPrevBlks().end());
      assert(erase_success(blk->GetPreDoms(), blk) >= 0);
      while(bit2 != blk->GetPrevBlks().end()){
	CBasicBlock *blk2 = *bit2++;
	if(blk2 != NULL){
	  if(blk->PreDomsJoin(blk2->GetPreDoms()) > 0)
	    success = true;
	}
	else{
	  if(blk->PreDomsJoin(root_pdoms) > 0)
	    success = true;
	}
      }
      nodup_insert(blk->GetPreDoms(), blk);
    }
  }

  // dominance tree
  bit = (cbt->GetBlockList()).begin();
  while(bit != (cbt->GetBlockList()).end()) {
    CBasicBlock *blk = *bit++;
    list<CBasicBlock*>::const_iterator bit2 = blk->GetPreDoms().begin();
    while(bit2 != blk->GetPreDoms().end()){
      CBasicBlock *blk2 = *bit2++;
      if((blk2 != NULL) && (blk2 != blk)){
	blk2->AddDoms(blk);
      }
    }
  }

  // dominance frontier
  cbt->ClearTempInfos();
  bit = (cbt->GetBlockList()).begin();
  while(bit != (cbt->GetBlockList()).end()) {
    CBasicBlock *blk = *bit++;
    blk->ComputeDF();
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
