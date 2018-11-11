#ifndef __SnuPL_OPT_H__
#define __SnuPL_OPT_H__

#include <iostream>
#include <list>
#include <vector>

#include "ir2.h"
#include "opt_pre.h"
#include "opt_rpm.h"
#include "opt_bba.h"
#include "opt_cpg.h"
#include "opt_tco.h"
#include "opt_ue.h"
#include "opt_ra.h"
#include "opt_ssa.h"
#include "symtab.h"


void full_optimize(int arch, CScope *m);

#endif // __SnuPL_OPT_H__
