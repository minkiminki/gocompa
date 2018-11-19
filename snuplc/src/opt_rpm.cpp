#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt_rpm.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Reigster Promotion
bool variable_used_block(CCodeBlock *cb, CSymbol* s){
  list<CTacInstr*>::const_iterator it = cb->GetInstr().begin();
  while(it != cb->GetInstr().end()){
    CTacInstr* instr = *it++;
    assert(instr != NULL);

    {
      CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
      if(dest != NULL){
	if(dest->GetSymbol() == s){
	  return true;
	}
      }
    }

    {
      CTacName* src1 = dynamic_cast<CTacName*>(instr->GetSrc(1));
      if(src1 != NULL){
	if(src1->GetSymbol() == s){
	  return true;
	}
      }
    }

    {
      CTacName* src2 = dynamic_cast<CTacName*>(instr->GetSrc(2));
      if(src2 != NULL){
	if(src2->GetSymbol() == s){
	  return true;
	}
      }
    }
  }

  return false;
}

bool variable_used_scope(CScope *m, CSymbol* s){
  if(variable_used_block(m->GetCodeBlock(), s)) return true;

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    if(variable_used_scope(*sit++, s)) return true;
  }

  return false;
}


void register_promotion_scope(CScope *m) {
  CSymtab *symtab = m->GetSymbolTable();
  assert(symtab != NULL);

  vector<CSymbol*> symbs = symtab->GetSymbols();
  vector<CSymbol*>::iterator sbit = symbs.begin();
  while (sbit != symbs.end()) {
    CSymbol *s = *sbit++;
    assert(s!=NULL);

    if(s->GetSymbolType() == stGlobal && s->GetData() == NULL){
      bool used = false;
      vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
      while (sit != m->GetSubscopes().end()) {
	if(variable_used_scope(*sit++, s)){
	  used = true; break;
	}
      }
      if(!used){
	s->SetSymbolType(stLocal);
      }
    }
  }


  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    register_promotion_scope(*sit++);
  }
  return;

}
