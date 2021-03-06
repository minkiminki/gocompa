#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <set>

#include "opt_tco.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Convert to SSA form
void ssa_in_block(CScope* m, CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  CBlockTable *cbt = cbp->GetBlockTable();
  assert(cbt != NULL);

  vector<CSymbol*> symbols = m->GetSymbolTable()->GetSymbols();

  vector<CSymbol*>::iterator sit = symbols.begin();
  while (sit != symbols.end()) {
    // insert phi
    CSymbol *s = *sit++;
    assert(s!=NULL);

    if((s->GetSymbolType() != stLocal) && (s->GetSymbolType() != stParam)) continue;

    bool success = false;
    cbt->ClearTempInfos();
    {
      list<CBasicBlock*>::const_iterator bit = cbt->GetBlockList().begin();
      list<CBasicBlock*> worklist;
      while (bit != cbt->GetBlockList().end()) {
	CBasicBlock* blk = *bit++;
	assert(blk!=NULL);
	if((blk->CheckAssign(s)) != NULL){
	  success = true;
	  blk->SetTempInfo(1);
	  nodup_insert(worklist, blk);
	}
      }
      if(!success) continue;

      while(true){
	CBasicBlock* nblk;
	if(list_pop(worklist, &nblk) > 0){
	  assert(nblk != NULL);
	  nblk->ComputePhi(worklist, s);
	}
	else{
	  break;
	}
      }
    }

    // rename variable
    map<CBasicBlock*, const CSymbol*> firstassigned;
    map<CBasicBlock*, const CSymbol*> lastassigned;
    set<const CSymbol*> newassigned;
    // newassigned.insert(s);
    list<CBasicBlock*>::const_iterator bit = cbt->GetBlockList().begin();
    while (bit != cbt->GetBlockList().end()) {
      CBasicBlock* blk = *bit++;
      const CSymbol* s_last = NULL;
      list<CTacInstr*>::const_iterator iit = blk->GetPhis().begin();
      while (iit != blk->GetPhis().end()) {
	CTacInstr* instr = *iit++;
	assert(instr->GetOperation() == opPhi);
	CTacName* n = dynamic_cast<CTacName*>(instr->GetDest());
	assert(n != NULL);
	if(n->GetSymbol() == s){
	  if(dynamic_cast<CTacReference*>(n) == NULL){
	    CTacTemp* _s_new = m->CreateTemp(n->GetSymbol()->GetDataType());
	    const CSymbol* s_new = _s_new->GetSymbol();
	    instr->SetDest(_s_new);
	    s_last = s_new;
	    firstassigned[blk] = s_new;
	    newassigned.insert(s_new);
	  }
	}
      }
      iit = blk->GetInstrs().begin();
      while (iit != blk->GetInstrs().end()) {
	CTacInstr* instr = *iit++;
	if(instr->GetOperation() == opAssign){
	  CTacName* n = dynamic_cast<CTacName*>(instr->GetDest());
	  assert(n != NULL);
	  if(n->GetSymbol() == s){
	    if(dynamic_cast<CTacReference*>(n) == NULL){
	      CTacTemp* _s_new = m->CreateTemp(n->GetSymbol()->GetDataType());
	      const CSymbol* s_new = _s_new->GetSymbol();
	      instr->SetDest(_s_new);
	      s_last = s_new;
	      newassigned.insert(s_new);
	    }
	  }
	}
      }
      if(s_last != NULL){
        lastassigned[blk] = s_last;
      }
    }

    {
      CBasicBlock *initblock = cbt->GetInitBlock();
      assert(initblock!=NULL);
      firstassigned[initblock] = s;
      if(lastassigned.find(initblock) == lastassigned.end()){
	lastassigned[initblock] = s;
      }
    }

    success = true;
    while(success){
      success = false;
      bit = cbt->GetBlockList().begin();
      while (bit != cbt->GetBlockList().end()) {
	CBasicBlock* blk = *bit++;
	if(lastassigned.find(blk) != lastassigned.end()){
	  list<CBasicBlock*>::const_iterator bit_next = (blk->GetNextBlks()).begin();
	  while(bit_next != (blk->GetNextBlks()).end()){
	    CBasicBlock* blk_next = *bit_next++;
	    if(blk_next == NULL) continue;
	    if(lastassigned.find(blk_next) == lastassigned.end()){
	      success = true;
	      lastassigned[blk_next] = lastassigned[blk];
	    }
	  }
	}
      }
    }

    {
      bit = cbt->GetBlockList().begin();
      while (bit != cbt->GetBlockList().end()) {
	CBasicBlock* blk = *bit++;
	list<CBasicBlock*>::const_iterator bit_next = (blk->GetNextBlks()).begin();
	while(bit_next != (blk->GetNextBlks()).end()){
	  CBasicBlock* blk_next = *bit_next++;
	  if(blk_next == NULL) continue;
	  if(firstassigned.find(blk_next) == firstassigned.end()){
	    firstassigned[blk_next] = lastassigned[blk];
	  }
	  else if(blk_next->GetTempInfo() >= 2){
	    list<CTacInstr*> _phis = blk_next->GetPhis();
	    assert(_phis.begin() != _phis.end());
	    CTacPhi *phi = dynamic_cast<CTacPhi*>(*(_phis.begin()));
	    assert(phi != NULL);
	    assert(phi->GetOperation() == opPhi);

	    CTacName* n = dynamic_cast<CTacName*>(phi->GetDest());
	    assert(n != NULL);
	    assert(newassigned.find(n->GetSymbol()) != newassigned.end());

	    if(blk_next->GetTempInfo() == 2){
	      CTacName* n2 = dynamic_cast<CTacName*>(phi->GetSrc(1));
	      assert(n2 != NULL);
	      n2->SetSymbol(lastassigned[blk]);
	      blk_next->SetTempInfo(3);

	      phi->SetSrcBlk(1, blk);

	      blk->AddBackPhi(n->GetSymbol(), lastassigned[blk]);
	    }
	    else if(blk_next->GetTempInfo() == 3){
	      CTacName* n2 = dynamic_cast<CTacName*>(phi->GetSrc(2));
	      assert(n2 != NULL);
	      n2->SetSymbol(lastassigned[blk]);
	      blk_next->SetTempInfo(4);

	      phi->SetSrcBlk(2, blk);

	      blk->AddBackPhi(n->GetSymbol(), lastassigned[blk]);
	    }
	    else{
	      assert(blk_next->GetTempInfo() == 4);
	    }

	  }
	}
      }
    }


    {
      bit = cbt->GetBlockList().begin();
      while (bit != cbt->GetBlockList().end()) {
    	CBasicBlock* blk = *bit++;
    	const CSymbol* s_last = firstassigned[blk];
    	list<CTacInstr*>::const_iterator iit = (blk->GetInstrs()).begin();

    	while(iit != (blk->GetInstrs()).end()){
    	  CTacInstr* instr = *iit++;
    	  assert(instr != NULL);

    	  {
    	    CTacName* n = dynamic_cast<CTacName*>(instr->GetSrc(1));
    	    if(n != NULL){
    	      if(n->GetSymbol() == s){
    		n->SetSymbol(s_last);
	      }
	    }
	  }

    	  {
    	    CTacName* n = dynamic_cast<CTacName*>(instr->GetSrc(2));
    	    if(n != NULL){
    	      if(n->GetSymbol() == s){
    		n->SetSymbol(s_last);
	      }
	    }
	  }

    	  {
    	    CTacName* n = dynamic_cast<CTacName*>(instr->GetDest());
	    if(n != NULL){
	      if(dynamic_cast<CTacReference*>(n) == NULL){
		if(newassigned.find(n->GetSymbol()) != newassigned.end()){
		  s_last = n->GetSymbol();
		}
	      }
	    }
	  }

	}

        // if(s_last != lastassigned[blk]){
	//   _P1;
	//   cout << blk <<endl;
	//   cout << s_last << endl;
	//   cout << lastassigned[blk] << endl;
	// }
	assert(s_last == lastassigned[blk]);
      }
    }

    // bit = cbt->GetBlockList().begin();
    // cout << "starttt---------------------\n" << s << endl;
    // while (bit != cbt->GetBlockList().end()) {
    //   CBasicBlock* blk = *bit++;
    //   cout << blk << endl;
    //   // if(lastassigned.find(blk) != lastassigned.end()){
    //   cout << lastassigned[blk] << endl;
    //   // }
    // }

  }
}

void ssa_in_scope(CScope *m){
  ssa_in_block(m, m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    ssa_in_scope(*sit++);
  }
  return;
}


// ********************************************************************** /
// ********************************************************************** /
// Translate out SSA
// void ssa_out_block(CCodeBlock *cb) {
//   CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
//   assert(cbp != NULL);
//   CBlockTable *cbt = cbp->GetBlockTable();
//   assert(cbt != NULL);
//   list<CBasicBlock*>::const_iterator bit = cbt->GetBlockList().begin();
//   while (bit != cbt->GetBlockList().end()) {
//     CBasicBlock* blk = *bit++;
//     assert(blk!=NULL);
//     assert(blk->GetInstrs().rbegin() != blk->GetInstrs().rend());
//     CTacInstr *instr = *(blk->GetInstrs().rbegin());
//     assert(instr != NULL);
//     list<CTacInstr*>::iterator fit = find(cbp->GetInstr().begin(), cbp->GetInstr().end(), instr);
//     assert(fit != cbp->GetInstr().end());
//     list<pair<const CSymbol*, const CSymbol*>>::const_iterator pit = blk->GetBackPhis().begin();
//     while(pit != blk->GetBackPhis().end()){
//       pair<const CSymbol*, const CSymbol*> spair = *pit++;
//       CTacInstr *_instr_new = new CTacInstr(opNop, new CTacTemp(spair.first),
// 					    new CTacTemp(spair.second), NULL);
//       CTacInstr_prime *instr_new = new CTacInstr_prime(_instr_new);
//       instr_new->SetOperation(opAssign);
//       instr_new->SetFromBlock(blk);
//       (blk->GetInstrs()).push_back(instr_new);
//       (cbp->GetInstr()).insert(fit, instr_new);
//       // (cbp->GetInstr()).insert(const_cast<CTacInstr*>(fit), instr_new);
//     }
//   }
// }

void ssa_out_scope(CScope *m){
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(m->GetCodeBlock());
  assert(cbp != NULL);
  cbp->SSA_out();
  // ssa_out_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    ssa_out_scope(*sit++);
  }
  return;
}
