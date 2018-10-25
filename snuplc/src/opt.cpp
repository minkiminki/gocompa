#include <cassert>
#include "opt.h"
using namespace std;


void full_optimize(CScope *m) {
  param_numbering_scope(m);
  clean_up_scope(m);
  to_ir_prime_scope(m);
  basic_block_analysis_scope(m);
  tail_call_optimization_scope(m);
  return;
}
