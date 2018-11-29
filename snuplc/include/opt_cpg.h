#ifndef __SnuPL_OPT_CPG_H__
#define __SnuPL_OPT_CPG_H__

#include <iostream>
#include <list>
#include <vector>

#include "ir2.h"
#include "symtab.h"


void uninitialized_vars_scope(CScope *m);
void constant_propagation_scope(CScope *m);

#endif // __SnuPL_OPT_CPG_H__
