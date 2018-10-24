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

void full_optimize(CScope *m) {
  to_ir_prime_scope(m);
  return;
}
