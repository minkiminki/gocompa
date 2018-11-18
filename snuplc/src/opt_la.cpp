#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt_la.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Dead Store Elimination
void dead_store_elimination_block(CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  CBlockTable *cbt = cbp->GetBlockTable();
  assert(cbt != NULL);

  map<CBasicBlock*, list<const CSymbol*>> defs;
  map<CBasicBlock*, list<const CSymbol*>> uses1;
  map<CBasicBlock*, list<const CSymbol*>> uses2;

  list<CBasicBlock*>::const_iterator bit = cbp->GetBlockTable()->GetBlockList().begin();
  while (bit != cbp->GetBlockTable()->GetBlockList().end()) {
    CBasicBlock* blk = *bit++;

    CBasicBlock* blk_prev1 = *blk->GetPrevBlks().begin();
    CBasicBlock* blk_prev2 = NULL;
    {
      if(next(blk->GetPrevBlks().begin()) != blk->GetPrevBlks().end())
	blk_prev2 = *next(blk->GetPrevBlks().begin());
    }

    list<const CSymbol*> blk_defs;
    list<const CSymbol*> blk_uses1;
    list<const CSymbol*> blk_uses2;

    list<CTacInstr*>::const_iterator it = blk->GetPhis().begin();
    while (it != blk->GetPhis().end()) {
      CTacPhi* instr = dynamic_cast<CTacPhi*>(*it++);
      assert(instr != NULL);

      if(instr->GetSrcBlk(1) == blk_prev1){
	assert(instr->GetSrcBlk(2)==blk_prev2);
	{
	  CTacName* src1 = dynamic_cast<CTacName*>(instr->GetSrc(1));
	  if(src1 != NULL){
	    const CSymbol* s_src1 = src1->GetSymbol();
	    assert(s_src1 != NULL);

	    if(s_src1->GetSymbolType() != stGlobal)
	      nodup_insert(blk_uses1, s_src1);
	  }
	}
	{
	  CTacName* src2 = dynamic_cast<CTacName*>(instr->GetSrc(2));
	  if(src2 != NULL){
	    const CSymbol* s_src2 = src2->GetSymbol();
	    assert(s_src2 != NULL);

	    if(s_src2->GetSymbolType() != stGlobal)
	      nodup_insert(blk_uses2, s_src2);
	  }
	}
      }
      else{
	assert(instr->GetSrcBlk(2)==blk_prev1);
	assert(instr->GetSrcBlk(1)==blk_prev2);
	{
	  CTacName* src1 = dynamic_cast<CTacName*>(instr->GetSrc(1));
	  if(src1 != NULL){
	    const CSymbol* s_src1 = src1->GetSymbol();
	    assert(s_src1 != NULL);

	    if(s_src1->GetSymbolType() != stGlobal)
	      nodup_insert(blk_uses2, s_src1);
	  }
	}
	{
	  CTacName* src2 = dynamic_cast<CTacName*>(instr->GetSrc(2));
	  if(src2 != NULL){
	    const CSymbol* s_src2 = src2->GetSymbol();
	    assert(s_src2 != NULL);

	    if(s_src2->GetSymbolType() != stGlobal)
	      nodup_insert(blk_uses1, s_src2);
	  }
	}
      }

      {
	CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
	assert(dest != NULL);
	const CSymbol* s_dest = dest->GetSymbol();
	assert(s_dest != NULL);

	if(s_dest->GetSymbolType() != stGlobal)
	  nodup_insert(blk_defs, s_dest);
      }
    }

    it = blk->GetInstrs().begin();
    while (it != blk->GetInstrs().end()) {
      CTacInstr* instr = *it++;
      assert(instr != NULL);

      {
	CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
	if(dest != NULL){
	  if(dynamic_cast<CTacReference*>(dest) != NULL){
	    const CSymbol* s_dest = dest->GetSymbol();
	    assert(s_dest != NULL);

	    if(s_dest->GetSymbolType() != stGlobal){
	      list<const CSymbol*>::iterator fit = find(blk_defs.begin(), blk_defs.end(),s_dest);
	      if(fit == blk_defs.end()){
		nodup_insert(blk_uses1, s_dest);
		nodup_insert(blk_uses2, s_dest);
	      }
	    }
	  }
	  else{
	    const CSymbol* s_dest = dest->GetSymbol();
	    assert(s_dest != NULL);
	    if(s_dest->GetSymbolType() != stGlobal){
	     nodup_insert(blk_defs, s_dest);
	    }
	  }
	}
      }
      {
	CTacName* src1 = dynamic_cast<CTacName*>(instr->GetSrc(1));
	if(src1 != NULL){
	  const CSymbol* s_src1 = src1->GetSymbol();
	  assert(s_src1 != NULL);

	  if(dynamic_cast<const CSymProc*>(s_src1) == NULL ){
	    if(s_src1->GetSymbolType() != stGlobal){
	      list<const CSymbol*>::iterator fit = find(blk_defs.begin(), blk_defs.end(), s_src1);
	      if(fit == blk_defs.end()){
		nodup_insert(blk_uses1, s_src1);
		nodup_insert(blk_uses2, s_src1);
	      }
	    }
	  }
	}
      }
      {
	CTacName* src2 = dynamic_cast<CTacName*>(instr->GetSrc(2));
	if(src2 != NULL){
	  const CSymbol* s_src2 = src2->GetSymbol();
	  assert(s_src2 != NULL);
	  if(s_src2->GetSymbolType() != stGlobal){
	    list<const CSymbol*>::iterator fit = find(blk_defs.begin(), blk_defs.end(),s_src2);
	    if(fit == blk_defs.end()){
	      nodup_insert(blk_uses1, s_src2);
	      nodup_insert(blk_uses2, s_src2);
	    }
	  }
	}
      }
    }

    defs[blk] = blk_defs;
    uses1[blk] = blk_uses1;
    uses2[blk] = blk_uses2;
  }

  // map<CBasicBlock*, list<const CSymbol*>> live_top;
  // map<CBasicBlock*, list<const CSymbol*>> live_bot;

  // bit = cbp->GetBlockTable()->GetBlockList().begin();
  // while (bit != cbp->GetBlockTable()->GetBlockList().end()) {
  //   CBasicBlock* blk = *bit++;
  //   live_top[blk] = uses
  //     blk_live_top;
  //   live_bot[blk] = blk_live_bot;
  // }

  bool success = true;
  while(success){
    success = false;

    // bit = cbp->GetBlockTable()->GetBlockList().begin();
    // while (bit != cbp->GetBlockTable()->GetBlockList().end()) {
    //   CBasicBlock* blk = *bit++;
    //   cout << "(" << blk->GetBlockNum() << " -";
    //   list<const CSymbol*>::iterator sit = uses1[blk].begin();
    //   while (sit != uses1[blk].end()) {
    // 	cout <<  " " << (*sit++)->GetName();;
    //   }
    //   cout << " |";
    //   sit = uses2[blk].begin();
    //   while (sit != uses2[blk].end()) {
    // 	cout <<  " " << (*sit++)->GetName();;
    //   }
    //   cout << ")" << endl;
    // }
    // cout << "-----------------------------------" << endl;

    bit = cbp->GetBlockTable()->GetBlockList().end();
    while (bit != cbp->GetBlockTable()->GetBlockList().begin()) {
      CBasicBlock* blk = *(--bit);

      list<const CSymbol*> uses1_next;
      list<const CSymbol*> uses2_next;

      list<CBasicBlock*>::iterator bit_next = blk->GetNextBlks().begin();
      while (bit_next != blk->GetNextBlks().end()) {
	CBasicBlock* blk_next = *bit_next++;

	// list<const CSymbol*> & uses_blk_next = uses[blk_next];

	if(blk_next == NULL) continue;

	if(*(blk_next->GetPrevBlks().begin()) == blk){
	  list_add(uses1_next, uses1[blk_next]);
	  list_add(uses2_next, uses1[blk_next]);
	}
	else if(*next(blk_next->GetPrevBlks().begin()) == blk){
	  list_add(uses1_next, uses2[blk_next]);
	  list_add(uses2_next, uses2[blk_next]);
	}

	// if(blk_next != NULL){
	//   list_add(uses_next1, uses[blk_next]);
	//   list_add(uses_next2, uses[blk_next]);
	// }
      }

      // list<const CSymbol*> & defs_blk = defs[blk];

      list_substract(uses1_next, defs[blk]);
      list_substract(uses2_next, defs[blk]);

      if(list_add(uses1[blk], uses1_next)) success = true;
      if(list_add(uses2[blk], uses2_next)) success = true;

    }
  }

  // bit = cbp->GetBlockTable()->GetBlockList().begin();
  // while (bit != cbp->GetBlockTable()->GetBlockList().end()) {
  //   CBasicBlock* blk = *bit++;
  //   cout << "(" << blk->GetBlockNum() << " -";
  //   list<const CSymbol*>::iterator sit = uses1[blk].begin();
  //   while (sit != uses1[blk].end()) {
  //     cout << " " << (*sit++)->GetName();
  //   }
  //   cout << " |";
  //   sit = uses2[blk].begin();
  //   while (sit != uses2[blk].end()) {
  //     cout << " " << (*sit++)->GetName();
  //   }
  //   cout << ")" << endl;
  // }

}

void dead_store_elimination_scope(CScope *m) {
  dead_store_elimination_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit = m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    dead_store_elimination_scope(*sit++);
  }
  return;
}


// ********************************************************************** /
// ********************************************************************** /
// Liveness Analysis
void liveness_analysis_block(CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  CBlockTable *cbt = cbp->GetBlockTable();
  assert(cbt != NULL);
}

void liveness_analysis_scope(CScope *m) {
  liveness_analysis_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    liveness_analysis_scope(*sit++);
  }
  return;
}
