#include <cassert>
#include "opt.h"
using namespace std;


void full_optimize(int arch, CScope *m) {
  param_numbering_scope(m);
  clean_up_scope(m); // DO NOT execute this after bba
  pointer_typing_scope(m);
  to_ir_prime_scope(m);
  basic_block_analysis_scope(m);
  tail_call_optimization_scope(arch, m);
  unused_elimination_scope(m);
  ssa_in_scope(m);
  unused_elimination_scope(m); // TODO : separate renumber
  // ssa_out_scope(m);
  return;
}
