#include <cassert>
#include "opt.h"
using namespace std;


void full_optimize(int arch, CScope *m) {
  param_numbering_scope(m);
  // parameter get number
  clean_up_scope(m); // DO NOT execute this after bba
  // now pointer variables have proper types
  dofs_inlining_scope(m);
  // DIM and DOFS inlined
  function_inlining_scope(m);
  unused_function_scope(m);
  pointer_typing_scope(m);
  register_promotion_scope(m);
  to_ir_prime_scope(m);
  // now it's ir_primex
  basic_block_analysis_scope(m);
  // get block info
  tail_call_optimization_scope(arch, m);
  // call -> tcall
  unused_elimination_scope(m);
  remove_nop_scope(m);
  ssa_in_scope(m);
  // in ssa form
  uninitialized_vars_scope(m);
  assign_param_scope(m);
  constant_propagation_scope(m);
  div_ref_scope(m);
  unused_elimination_scope(m);
  combine_blocks_scope(m);
  dead_store_elimination_scope(m);
  remove_var_scope(m);
  liveness_analysis_scope(m);
  register_allocation_scope(arch, m);
  // base regiseter and offset set
  ssa_out_scope(m);
  // remove phi
  remove_nop_scope(m);
  remove_goto_scope(m);
  remove_label_scope(m);
  instruction_renumber_scope(m);
  return;
}
