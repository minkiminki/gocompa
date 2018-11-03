#ifndef __SnuPL_OPT_PRE_H__
#define __SnuPL_OPT_PRE_H__

#include <iostream>
#include <list>
#include <vector>

#include "ir2.h"
#include "symtab.h"


void pointer_typing_scope(CScope *m);
void param_numbering_scope(CScope *m);
void clean_up_scope(CScope *m);
void to_ir_prime_scope(CScope *m);

#endif // __SnuPL_OPT_PRE_H__
