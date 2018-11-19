#ifndef __SnuPL_OPT_UE_H__
#define __SnuPL_OPT_UE_H__

#include <iostream>
#include <list>
#include <vector>

#include "ir2.h"
#include "symtab.h"


void remove_var_scope(CScope *m);
void remove_label_scope(CScope *m);
void remove_goto_scope(CScope *m);
void remove_nop_scope(CScope *m);
void instruction_renumber_scope(CScope *m);
void unused_elimination_scope(CScope *m);

#endif // __SnuPL_OPT_UE_H__
