#ifndef __SnuPL_OPT_SSA_H__
#define __SnuPL_OPT_SSA_H__

#include <iostream>
#include <list>
#include <vector>

#include "ir2.h"
#include "symtab.h"


void ssa_in_scope(CScope *m);
void ssa_out_scope(CScope *m);

#endif // __SnuPL_OPT_SSA_H__
