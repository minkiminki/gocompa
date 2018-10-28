#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt_ra.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Register Alloction

void register_allocation_block(int arch, CSymtab *symtab, CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);
  vector<CSymbol*> slist = symtab->GetSymbols();

	// compute stack offset
	int callee_save = 5;
	int param_ofs = -(callee_save)*8;
	int local_ofs = param_ofs;
  size_t sp_align = 8; // stack pointer alignment
  size_t size = 0;
	int param_num = 0;

	// for first iteration, assign regs for param(<6) and compute param number
  for (size_t i=0; i<slist.size(); i++) {
    CSymbol *s = slist[i];
    const CType *t = s->GetDataType();

    ESymbolType st = s->GetSymbolType();

    if (st == stParam) {
			if(++param_num > 6){
			} else {
				CSymParam *p = dynamic_cast<CSymParam*>(s);
				assert(p != NULL);

				p->SetBaseRegister("%rbp");
				p->SetOffset(param_ofs - (p->GetIndex()+1)*8);
			}
    }
  }

	//set locals after param values, set param>6 on eariler stack :) :(
	if(param_num > 6){
		local_ofs -= 6 * 8;
		param_ofs = 16 + (param_num - 6)*8;
	}
	else
		local_ofs -= param_num * 8;

	// iterate again to assign locals and params(>6) after get param num
  for (size_t i=0; i<slist.size(); i++) {
    CSymbol *s = slist[i];
    const CType *t = s->GetDataType();

    ESymbolType st = s->GetSymbolType();

    if (st == stLocal) {
      int ssize = GetSize_prime(t);
      int align = GetAlign_prime(t);
/*			if(t->IsPointer()) {
				ssize = 8;
				align = 8;
			}
*/
      local_ofs -= ssize;

      if ((align > 1) && (local_ofs % align != 0)) {
        // align towards smaller addresses
        align = (local_ofs - align +1) / align * align - local_ofs;
      } else {
        align = 0;
      }

      size += ssize - align;      // align is negative
      local_ofs += align;

      s->SetBaseRegister("%rbp");
      s->SetOffset(local_ofs);
		} else if (st == stParam) {
			if(param_num > 6){
				CSymParam *p = dynamic_cast<CSymParam*>(s);
				assert(p != NULL);

				p->SetBaseRegister("%rbp");
				p->SetOffset(param_ofs - (p->GetIndex()-5)*8);
			}
    }
	}
  size = (size + sp_align-1) / sp_align * sp_align;

	cbp->SetParamNum(param_num);
  cbp->SetStackSize(size);
}

void register_allocation_scope(int arch, CScope *m) {
  register_allocation_block(arch, m->GetSymbolTable(), m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    register_allocation_scope(arch, *sit++);
  }
  return;
}
