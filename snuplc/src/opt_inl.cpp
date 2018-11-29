h#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include <cassert>

#include "opt_inl.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Function Inlining

int get_function(map<string, pair<const list<CTacInstr*>, vector<CSymbol*>>> & functions) {



  int success = 0;
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  set<const CSymbol*> defs;

  list<CBasicBlock*>::const_iterator bit = cbp->GetBlockTable()->GetBlockList().begin();
  while (bit != cbp->GetBlockTable()->GetBlockList().end()) {
    CBasicBlock* blk = *bit++;

    list<CTacInstr*>::const_iterator it = blk->GetPhis().begin();
    while (it != blk->GetPhis().end()) {
      CTacInstr* instr = *it++;
      assert(instr != NULL);
    }
  }

}


void function_inlining_scope(CScope *m) {

  map<string, pair<const list<CTacInstr*>, vector<CSymbol*>>> functions;

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    CScope * sc = *sit++;



  remove_var_block(m->GetSymbolTable(), m->GetCodeBlock());





  }




  function_inlining_block(m->GetCodeBlock());
  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    function_inlining_scope(*sit++);
  }
  return;
}

void unused_function_scope(CSCope *m) {

}
