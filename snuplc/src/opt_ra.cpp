#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt_ra.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Register Alloction
size_t ComputeStackOffsets(CSymtab *symtab,
			   int param_ofs,int local_ofs)
{
  assert(symtab != NULL);
  vector<CSymbol*> slist = symtab->GetSymbols();

  size_t sp_align = 4; // stack pointer alignment
  size_t size = 0;

  for (size_t i=0; i<slist.size(); i++) {
    CSymbol *s = slist[i];
    const CType *t = s->GetDataType();

    ESymbolType st = s->GetSymbolType();

    if (st == stLocal) {
      int ssize = t->GetSize();
      int align = t->GetAlign();

      local_ofs -= ssize;

      if ((align > 1) && (local_ofs % align != 0)) {
        // align towards smaller addresses
        align = (local_ofs - align +1) / align * align - local_ofs;
      } else {
        align = 0;
      }

      size += ssize - align;      // align is negative
      local_ofs += align;

      s->SetBaseRegister("%ebp");
      s->SetOffset(local_ofs);

    } else if (st == stParam) {
      CSymParam *p = dynamic_cast<CSymParam*>(s);
      assert(p != NULL);

      p->SetBaseRegister("%ebp");
      p->SetOffset(param_ofs + p->GetIndex()*4);
    }
  }
  size = (size + sp_align-1) / sp_align * sp_align;
  return size;
}

void register_allocation_block(int arch, CSymtab *st, CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);
  // TODO depende on arch
  switch(arch){
  case 32: cbp->SetStackSize(ComputeStackOffsets(st, 8, -12)); break;
  case 64: cbp->SetStackSize(ComputeStackOffsets(st, 8, -12)); // TODO - FIX IT!!
    break;
  default: assert(false);
  }
}

void register_allocation_scope(int arch, CScope *m) {
  register_allocation_block(arch, m->GetSymbolTable(), m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    register_allocation_scope(arch, *sit++);
  }
  return;
}
