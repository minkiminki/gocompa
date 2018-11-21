#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <set>

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

  set<const CSymbol*> needed;

  bool success = true;

  while(success){
    success = false;
    list<CBasicBlock*>::const_reverse_iterator bit = cbp->GetBlockTable()->GetBlockList().rbegin();
    while (bit != cbp->GetBlockTable()->GetBlockList().rend()) {
      CBasicBlock* blk = *bit++;

      list<CTacInstr*>::const_reverse_iterator it = blk->GetInstrs().rbegin();
      while (it != blk->GetInstrs().rend()) {
	CTacInstr* instr = *it++;
	assert(instr != NULL);

	CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
	CTacName* src1 = dynamic_cast<CTacName*>(instr->GetSrc(1));
	CTacName* src2 = dynamic_cast<CTacName*>(instr->GetSrc(2));

	if(dest != NULL){
	  if(dynamic_cast<CTacReference*>(instr->GetDest()) != NULL){
	    if(dest->GetSymbol()->GetSymbolType() == stLocal){
	      if(needed.insert(dest->GetSymbol()).second)
		success = true;
	    }
	    if(src1 != NULL && src1->GetSymbol()->GetSymbolType() == stLocal){
	      if(needed.insert(src1->GetSymbol()).second)
		success = true;
	    }
	    if(src2 != NULL && src2->GetSymbol()->GetSymbolType() == stLocal){
	      if(needed.insert(src2->GetSymbol()).second)
		success = true;
	    }
	  }
	  else if(dest->GetSymbol()->GetSymbolType() == stGlobal || needed.find(dest->GetSymbol()) != needed.end()){
	    if(src1 != NULL && src1->GetSymbol()->GetSymbolType() == stLocal){
	      if(needed.insert(src1->GetSymbol()).second)
		success = true;
	    }
	    if(src2 != NULL && src2->GetSymbol()->GetSymbolType() == stLocal){
	      if(needed.insert(src2->GetSymbol()).second)
		success = true;
	    }
	  }
	}
	else if(instr->GetOperation() == opParam || instr->GetOperation() == opReturn){
	  if(src1 != NULL && src1->GetSymbol()->GetSymbolType() == stLocal){
	    if(needed.insert(src1->GetSymbol()).second)
	      success = true;
	  }
	}
	else if(instr->IsBranch()){
	  if(src1 != NULL && src1->GetSymbol()->GetSymbolType() == stLocal){
	    if(needed.insert(src1->GetSymbol()).second)
	      success = true;
	  }
	  if(src2 != NULL && src2->GetSymbol()->GetSymbolType() == stLocal){
	    if(needed.insert(src2->GetSymbol()).second)
	      success = true;
	  }
	}
      }

      it = blk->GetPhis().rbegin();
      while (it != blk->GetPhis().rend()) {
	CTacInstr* instr = *it++;
	assert(instr != NULL);

	CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
	CTacName* src1 = dynamic_cast<CTacName*>(instr->GetSrc(1));
	CTacName* src2 = dynamic_cast<CTacName*>(instr->GetSrc(2));

	if(needed.find(dest->GetSymbol()) != needed.end()){
	  if(src1 != NULL && src1->GetSymbol()->GetSymbolType() == stLocal){
	    if(needed.insert(src1->GetSymbol()).second)
	      success = true;
	  }
	  if(src2 != NULL && src2->GetSymbol()->GetSymbolType() == stLocal){
	    if(needed.insert(src2->GetSymbol()).second)
	      success = true;
	  }
	}
      }
    }
  }

  list<CBasicBlock*>::const_reverse_iterator bit = cbp->GetBlockTable()->GetBlockList().rbegin();
  while (bit != cbp->GetBlockTable()->GetBlockList().rend()) {
    CBasicBlock* blk = *bit++;

    list<CTacInstr*>::const_iterator it = blk->GetInstrs().begin();
    while (it != blk->GetInstrs().end()) {
      CTacInstr* instr = *it++;
      assert(instr != NULL);

      CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
      if(dest != NULL && dest->GetSymbol()->GetSymbolType() == stLocal && needed.find(dest->GetSymbol()) == needed.end()){
	// cout << instr << endl;
	// _P1;
	if(instr->GetOperation() != opCall && instr->GetOperation() != opTailCall){
	  instr->SetSrc(0, NULL);
	  instr->SetSrc(1, NULL);
	  instr->SetOperation(opNop);
	}
	instr->SetDest(NULL);
      }
    }

    it = blk->GetPhis().begin();
    while (it != blk->GetPhis().end()) {
      list<CTacInstr*>::const_iterator it_temp = it++;
      CTacInstr* instr = *it_temp;
      assert(instr != NULL);

      CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
      if(needed.find(dest->GetSymbol()) == needed.end()){
	// cout << instr << endl;
	// _P1;
	(blk->GetPhis()).erase(it_temp);
      }
    }
  }
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

  Liveness *liveness = cbt->GetLiveness();
  assert(liveness != NULL);

  map<CBasicBlock*, list<const CSymbol*>> defs;
  map<CBasicBlock*, list<const CSymbol*>> uses1 = liveness->GetUses(1);
  map<CBasicBlock*, list<const CSymbol*>> uses2 = liveness->GetUses(2);
  map<CBasicBlock*, list<const CSymbol*>> uses_below;

  uses1.clear();
  uses2.clear();

  const CSymbol** param_regs = liveness->GetParamRegs();
  // param_regs[0] = new CSymbol("%rdi", stRegister, nulltyp);
  // param_regs[1] = new CSymbol("%rsi", stRegister, nulltyp);
  // param_regs[2] = new CSymbol("%rdx", stRegister, nulltyp);
  // param_regs[3] = new CSymbol("%rcx", stRegister, nulltyp);
  // param_regs[4] = new CSymbol("%r8", stRegister, nulltyp);
  // param_regs[5] = new CSymbol("%r9", stRegister, nulltyp);

  const CSymbol* caller_save1 = liveness->GetCallerSave(1);
  const CSymbol* caller_save2 = liveness->GetCallerSave(2);
  // const CSymbol* caller_save1 = new CSymbol("%r10", stRegister, nulltyp);
  // const CSymbol* caller_save2 = new CSymbol("%r11", stRegister, nulltyp);

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

	    if(s_src1->GetSymbolType() != stGlobal && !(s_src1->GetDataType()->IsArray()))
	      nodup_insert(blk_uses1, s_src1);
	  }
	}
	{
	  CTacName* src2 = dynamic_cast<CTacName*>(instr->GetSrc(2));
	  if(src2 != NULL){
	    const CSymbol* s_src2 = src2->GetSymbol();
	    assert(s_src2 != NULL);

	    if(s_src2->GetSymbolType() != stGlobal && !(s_src2->GetDataType()->IsArray()))
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

	    if(s_src1->GetSymbolType() != stGlobal && !(s_src1->GetDataType()->IsArray()))
	      nodup_insert(blk_uses2, s_src1);
	  }
	}
	{
	  CTacName* src2 = dynamic_cast<CTacName*>(instr->GetSrc(2));
	  if(src2 != NULL){
	    const CSymbol* s_src2 = src2->GetSymbol();
	    assert(s_src2 != NULL);

	    if(s_src2->GetSymbolType() != stGlobal && !(s_src2->GetDataType()->IsArray()))
	      nodup_insert(blk_uses1, s_src2);
	  }
	}
      }

      {
	CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
	assert(dest != NULL);
	const CSymbol* s_dest = dest->GetSymbol();
	assert(s_dest != NULL);

	if(s_dest->GetSymbolType() != stGlobal && !(s_dest->GetDataType()->IsArray()))
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

	    if(s_dest->GetSymbolType() != stGlobal && !(s_dest->GetDataType()->IsArray())){
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
	    if(s_dest->GetSymbolType() != stGlobal && !(s_dest->GetDataType()->IsArray())){
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
	    if(s_src1->GetSymbolType() != stGlobal && !(s_src1->GetDataType()->IsArray())){
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
	  if(s_src2->GetSymbolType() != stGlobal && !(s_src2->GetDataType()->IsArray())){
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

    list<const CSymbol*> blk_uses_below;
    uses_below[blk] = blk_uses_below;
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
    //   list<const CSymbol*>::iterator sit = uses_below[blk].begin();
    //   while (sit != uses_below[blk].end()) {
    // 	cout <<  " " << (*sit++)->GetName();;
    //   }

    //   // cout << " |";
    //   // sit = uses2[blk].begin();
    //   // while (sit != uses2[blk].end()) {
    //   // 	cout <<  " " << (*sit++)->GetName();;
    //   // }

    //   cout << ")" << endl;
    // }
    // cout << "-----------------------------------" << endl;

    bit = cbp->GetBlockTable()->GetBlockList().end();
    while (bit != cbp->GetBlockTable()->GetBlockList().begin()) {
      CBasicBlock* blk = *(--bit);

      // list<const CSymbol*> uses1_next;
      // list<const CSymbol*> uses2_next;

      list<CBasicBlock*>::iterator bit_next = blk->GetNextBlks().begin();
      while (bit_next != blk->GetNextBlks().end()) {
	CBasicBlock* blk_next = *bit_next++;

	// list<const CSymbol*> & uses_blk_next = uses[blk_next];

	if(blk_next == NULL) continue;

	if(*(blk_next->GetPrevBlks().begin()) == blk){
	  list_add(uses_below[blk], uses1[blk_next]);
	  // list_add(uses1_next, uses1[blk_next]);
	  // list_add(uses2_next, uses1[blk_next]);
	}
	else if(*next(blk_next->GetPrevBlks().begin()) == blk){
	  list_add(uses_below[blk], uses2[blk_next]);
	  // list_add(uses1_next, uses2[blk_next]);
	  // list_add(uses2_next, uses2[blk_next]);
	}

	// if(blk_next != NULL){
	//   list_add(uses_next1, uses[blk_next]);
	//   list_add(uses_next2, uses[blk_next]);
	// }
      }

      // list<const CSymbol*> & defs_blk = defs[blk];

      list<const CSymbol*> uses_next = uses_below[blk];
      list_substract(uses_next, defs[blk]);
      // list_substract(uses1_next, defs[blk]);
      // list_substract(uses2_next, defs[blk]);

      if(list_add(uses1[blk], uses_next)) success = true;
      if(list_add(uses2[blk], uses_next)) success = true;
      // if(list_add(uses1[blk], uses1_next)) success = true;
      // if(list_add(uses2[blk], uses2_next)) success = true;

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
  // cout << "------------------------------------" << endl;


  bit = cbp->GetBlockTable()->GetBlockList().begin();
  while (bit != cbp->GetBlockTable()->GetBlockList().end()) {
    CBasicBlock* blk = *bit++;
    list<const CSymbol*> live_vars = uses_below[blk];

    list<CTacInstr*>::iterator it = blk->GetInstrs().end();
    while (it != blk->GetInstrs().begin()) {
      CTacInstr_prime* instr = dynamic_cast<CTacInstr_prime*>(*--it);

      {
	CTacName* src1 = dynamic_cast<CTacName*>(instr->GetSrc(1));
	if(src1 != NULL){
	  const CSymbol* s_src1 = src1->GetSymbol();
	  assert(s_src1 != NULL);

	  if(dynamic_cast<const CSymProc*>(s_src1) == NULL ){
	    if(s_src1->GetSymbolType() != stGlobal && !(s_src1->GetDataType()->IsArray())){

	      nodup_insert(live_vars, s_src1);
	      // list<const CSymbol*>::iterator fit = find(live_vars.begin(), live_vars.end(), s_src1);
	      // if(fit == live_vars.end()){
	      // 	nodup_insert(live_vars, s_src1);
	      // }

	    }
	  }
	}
      }
      {
	CTacName* src2 = dynamic_cast<CTacName*>(instr->GetSrc(2));
	if(src2 != NULL){
	  const CSymbol* s_src2 = src2->GetSymbol();
	  assert(s_src2 != NULL);
	  if(s_src2->GetSymbolType() != stGlobal && !(s_src2->GetDataType()->IsArray())){
	    // list<const CSymbol*>::iterator fit = find(live_vars.begin(), live_vars.end(),s_src2);
	    // if(fit == live_vars.end()){
	    //   nodup_insert(live_vars, s_src2);
	    // }

	    nodup_insert(live_vars, s_src2);
	  }
	}
      }
      {
	CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
	if(dest != NULL){
	  if(dynamic_cast<CTacReference*>(dest) != NULL){
	    const CSymbol* s_dest = dest->GetSymbol();
	    assert(s_dest != NULL);

	    if(s_dest->GetSymbolType() != stGlobal && !(s_dest->GetDataType()->IsArray())){
	      // list<const CSymbol*>::iterator fit = find(live_vars.begin(), live_vars.end(),s_dest);
	      // if(fit == live_vars.end()){
	      // 	nodup_insert(live_vars, s_dest);
	      // }
	      nodup_insert(live_vars, s_dest);
	    }
	  }
	  else{
	    const CSymbol* s_dest = dest->GetSymbol();
	    assert(s_dest != NULL);
	    if(s_dest->GetSymbolType() != stGlobal && !(s_dest->GetDataType()->IsArray())){

	      erase_success(live_vars, s_dest);

	      // if(erase_success(live_vars, s_dest) < 0){
	      // 	// cout << s_dest << " isn't used" << endl;
	      // 	// _P1;
	      // 	// todo: DSE!!

	      // 	if(instr->GetOperation() != opCall && instr->GetOperation() != opTailCall){
	      // 	  instr->SetSrc(0, NULL);
	      // 	  instr->SetSrc(1, NULL);
	      // 	  instr->SetOperation(opNop);
	      // 	}
	      // 	instr->SetDest(NULL);

	      // 	dse_success = true;
	      // }

	    }
	  }
	}
      }

      if(instr->GetOperation() == opCall || instr->GetOperation() == opTailCall){
	CTacName *n = dynamic_cast<CTacName*>(instr->GetSrc(1));
	assert(n != NULL);
	const CSymProc *proc = dynamic_cast<const CSymProc*>(n->GetSymbol());
	assert(proc != NULL);

	int num = proc->GetNParams();
        num = (num > 6) ? 6 : num;

	switch(num){
	case 6: nodup_insert(live_vars, param_regs[5]);
	case 5: nodup_insert(live_vars, param_regs[4]);
	case 4: nodup_insert(live_vars, param_regs[3]);
	case 3: nodup_insert(live_vars, param_regs[2]);
	case 2: nodup_insert(live_vars, param_regs[1]);
	case 1: nodup_insert(live_vars, param_regs[0]);
	}
      }
      else if(instr->GetOperation() == opParam){
	CTacConst *n = dynamic_cast<CTacConst*>(instr->GetDest());
	assert(n != NULL);
	int num = n->GetValue();
	assert(erase_success(live_vars, param_regs[num-1]) >= 0);
      }

      instr->SetLiveVars(live_vars);

      if(instr->GetOperation() == opCall || instr->GetOperation() == opTailCall){
	instr->GetLiveVars().push_back(caller_save1);
	instr->GetLiveVars().push_back(caller_save2);
      }
    }

    it = blk->GetPhis().begin();
    while (it != blk->GetPhis().end()) {
      list<CTacInstr*>::iterator it_temp = it++;
      CTacInstr_prime* instr = dynamic_cast<CTacInstr_prime*>(*it_temp);
      assert(instr != NULL);
      CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
      assert(dest != NULL);
      const CSymbol* s_dest = dest->GetSymbol();
      assert(s_dest != NULL);
      erase_success(live_vars, s_dest);
      // if(erase_success(live_vars, s_dest) < 0){
      // 	// cout << s_dest << " isn't used" << endl;
      // 	// _P1;
      // 	// TODO: DSE!!

      // 	(blk->GetPhis()).erase(it_temp);
      // 	dse_success = true;

      // }
    }
  }
}

void liveness_analysis_scope(CScope *m) {
  liveness_analysis_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit = m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    liveness_analysis_scope(*sit++);
  }
  return;
}
