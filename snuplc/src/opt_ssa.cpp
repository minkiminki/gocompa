#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt_tco.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Convert to SSA form
void ssa_in_block(CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  CBlockTable *cbt = cbp->GetBlockTable();
  assert(cbt != NULL);

  ADMIT;
}

void ssa_in_scope(CScope *m){
  ssa_in_block(m->GetCodeBlock());

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
  ssa_in_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    ssa_in_scope(*sit++);
  }
  return;
}
