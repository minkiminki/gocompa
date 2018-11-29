#ifndef __SnuPL_OPT_LA_H__
#define __SnuPL_OPT_LA_H__

#include <iostream>
#include <list>
#include <vector>

#include "ir2.h"
#include "symtab.h"


void dead_store_elimination_scope(CScope *m);
void liveness_analysis_scope(CScope *m);
void assign_param_scope(CScope *m);

#endif // __SnuPL_OPT_LA_H__
