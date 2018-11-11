#ifndef __SnuPL_OPT_BBA_H__
#define __SnuPL_OPT_BBA_H__

#include <iostream>
#include <list>
#include <vector>

#include "ir2.h"
#include "symtab.h"


void remove_unreachable_block(CCodeBlock *cb);
void combine_blocks_scope(CScope *m);
void basic_block_analysis_scope(CScope *m);

#endif // __SnuPL_OPT_BBA_H__
