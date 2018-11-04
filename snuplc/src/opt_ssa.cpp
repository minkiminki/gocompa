#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt_tco.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Convert to SSA form
void ssa_in_block(CSymtab* st, CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  CBlockTable *cbt = cbp->GetBlockTable();
  assert(cbt != NULL);

  vector<CSymbol*> symbols = st->GetSymbols();

  vector<CSymbol*>::iterator sit = symbols.begin();
  while (sit != symbols.end()) {
    CSymbol *s = *sit++;
    assert(s!=NULL);
    if((s->GetSymbolType() == stLocal) || (s->GetSymbolType() == stParam)){
      list<CBasicBlock*>::const_iterator bit = cbt->GetBlockList().begin();
      list<CBasicBlock*> worklist;
      while (bit != cbt->GetBlockList().end()) {
	CBasicBlock* blk = *bit++;
	assert(blk!=NULL);
	if((blk->CheckAssign(s)) != NULL){
	  blk->SetTempInfo(1);
	  nodup_insert(worklist, blk);
	}
      }

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
  }
}

void ssa_in_scope(CScope *m){
  ssa_in_block(m->GetSymbolTable(), m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    ssa_in_scope(*sit++);
  }
  return;
}


// ********************************************************************** /
// ********************************************************************** /
// Translate out SSA
void ssa_out_block(CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  CBlockTable *cbt = cbp->GetBlockTable();
  assert(cbt != NULL);

  ADMIT;
}

void ssa_out_scope(CScope *m){
  ssa_out_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    ssa_in_scope(*sit++);
  }
  return;
}
