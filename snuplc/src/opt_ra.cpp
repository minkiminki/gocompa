#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <map>

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
  // int callee_save = 5;
  int maxreg = cbp->GetBlockTable()->GetLiveness()->GetMax();

  int callee_save;

  if(maxreg > rgMAX) callee_save = 5;
  else if(maxreg < rgRBX) callee_save = 0;
  else callee_save = maxreg - rgR11;

  int param_ofs = -(callee_save)*8;
  int local_ofs = param_ofs;
  size_t sp_align = 8; // stack pointer alignment
  size_t size = 0;
  int param_num = 0;


  // // for first iteration, assign regs for param(<6) and compute param number
  // for (size_t i=0; i<slist.size(); i++) {
  //   CSymbol *s = slist[i];
  //   const CType *t = s->GetDataType();

  //   ESymbolType st = s->GetSymbolType();

  //   if (st == stParam) {
  //     if(++param_num > 6){
  //     }
  //     else {
  // 	CSymParam *p = dynamic_cast<CSymParam*>(s);
  // 	assert(p != NULL);

  // 	p->SetBaseRegister("");

  // 	// p->SetBaseRegister("%rbp");
  // 	p->SetOffset(param_ofs - (p->GetIndex()+1)*8);
  //     }
  //   }
  // }
  param_num = cbp->GetBlockTable()->GetLiveness()->GetParamNum();

  //set locals after param values, set param>6 on eariler stack :) :(
  /*
  if(param_num > 6){
    local_ofs -= 6 * 8;
    param_ofs = 16 + (param_num - 6)*8;
  }
  else
    local_ofs -= param_num * 8;
  */

  /* testing ... */
  map<ERegister, int> reg_to_stack;
  map<const CSymbol*, ERegister> & symb_to_reg = cbp->GetBlockTable()->GetLiveness()->GetAllocated();

  // iterate again to assign locals and params(>6) after get param num
  for (size_t i=0; i<slist.size(); i++) {
    CSymbol *s = slist[i];
    const CType *t = s->GetDataType();

    ESymbolType st = s->GetSymbolType();

    if (st == stLocal || st == stParam) {

      if(s->GetDataType()->IsArray()){
        int ssize = GetSize_prime(t);
        int align = GetAlign_prime(t);
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
      }
      else{

        if(symb_to_reg.find(s) == symb_to_reg.end()){
          cout << s << endl;
          _P1;
        }

        ERegister e = symb_to_reg[s];
        if(e > rgMAX){
          if(reg_to_stack.find(e) == reg_to_stack.end()){
            int ssize = 8;
            int align = 8;
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
            // cout << ERegName[e] << " : " << local_ofs << endl;
            reg_to_stack[e] = local_ofs;
          }
          else{
            // local_ofs = reg_to_stack[e];
            s->SetBaseRegister("%rbp");
            s->SetOffset(reg_to_stack[e]);
          }
        }
      }

    }
    // else if (st == stParam) {
    //   if(param_num > 6){
    // 	CSymParam *p = dynamic_cast<CSymParam*>(s);
    // 	assert(p != NULL);

    // 	p->SetBaseRegister("");
    // 	// p->SetBaseRegister("%rbp");
    // 	p->SetOffset(param_ofs - (p->GetIndex()-5)*8);
    //   }
    // }

    // cout << s << endl;
    // cout << local_ofs << endl;


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
