#ifndef __SnuPL_OPT_H__
#define __SnuPL_OPT_H__

#include <iostream>
#include <list>
#include <vector>

#include "ir2.h"
#include "opt_pre.h"
#include "opt_bba.h"
#include "opt_tco.h"
#include "symtab.h"


void full_optimize(CScope *m);

#endif // __SnuPL_OPT_H__
