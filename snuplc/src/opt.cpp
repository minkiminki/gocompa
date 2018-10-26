#include <cassert>
#include "opt.h"
using namespace std;


void full_optimize(int arch, CScope *m) {
  param_numbering_scope(m);
  clean_up_scope(m);
  to_ir_prime_scope(m);
  basic_block_analysis_scope(m);
  tail_call_optimization_scope(arch, m);
  unused_elimination_scope(m);
  register_allocation_scope(arch, m);
  return;
}
