#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include <map>
#include <cassert>
#include <string>
#include "opt_inl.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Function Inlining

bool is_rte(string s){
  if(s.compare("ReadInt") == 0) return true;
  if(s.compare("WriteInt") == 0) return true;
  if(s.compare("WriteStr") == 0) return true;
  if(s.compare("WriteChar") == 0) return true;
  if(s.compare("WriteLn") == 0) return true;
  return false;
}

CTac* convert_tac(map<const CSymbol*, const CSymbol*> &new_symbs,
		  map<CTacInstr*, CTacInstr*> &new_lbs,
		  CTac* src)
{
  if(src == NULL) return NULL;
  CTac *src_new;

  if(dynamic_cast<CTacInstr*>(src) != NULL){
    src_new = new_lbs[dynamic_cast<CTacInstr*>(src)];
  }
  else if(dynamic_cast<CTacConst*>(src) != NULL){
    src_new = new CTacConst(dynamic_cast<CTacConst*>(src)->GetValue());
  }
  else if(dynamic_cast<CTacReference*>(src) != NULL){
    const CSymbol *s = dynamic_cast<CTacName*>(src)->GetSymbol();
    const CSymbol *s_new;
    const CSymbol *s_deref = dynamic_cast<CTacReference*>(src)->GetDerefSymbol();
    const CSymbol *s_deref_new;

    if(s->GetSymbolType() == stGlobal){
      s_new = s;
    }
    else if(s->GetSymbolType() == stProcedure){
      s_new = s;
    }
    else{
      s_new = new_symbs[s];
    }

    if(s_deref->GetSymbolType() == stGlobal){
      s_deref_new = s_deref;
    }
    else if(s_deref->GetSymbolType() == stProcedure){
      s_deref_new = s_deref;
    }
    else{
      s_deref_new = new_symbs[s_deref];
    }

    src_new = new CTacReference(s_new, s_deref_new);
  }
  else{
    assert(dynamic_cast<CTacName*>(src) != NULL);

    const CSymbol *s = dynamic_cast<CTacName*>(src)->GetSymbol();
    const CSymbol *s_new;

    if(s->GetSymbolType() == stGlobal){
      s_new = s;
    }
    else if(s->GetSymbolType() == stProcedure){
      s_new = s;
    }
    else{
      s_new = new_symbs[s];
    }

    src_new = new CTacName(s_new);
  }

  return src_new;
}


void get_functions(map<string, list<CTacInstr*>> &functions,
		   map<string, vector<CSymbol*>> &symbols,
		   map<string, list<CTacInstr*>> &labels, CScope *m) {
  vector<CScope*>::const_iterator sit = m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    CScope * sc = *sit++;
    assert(sc != NULL);
    string n = sc->GetName();
    cout << n << endl;
    functions[n] = sc->GetCodeBlock()->GetInstr();
    symbols[n] = sc->GetSymbolTable()->GetSymbols();
    {
      list<CTacInstr*> lbs;
      CCodeBlock *cb = m->GetCodeBlock();
      list <CTacInstr*> &instrs = const_cast<list <CTacInstr*> &>(cb->GetInstr());
      list<CTacInstr*>::iterator it = instrs.begin();
      while (it != instrs.end()) {
	CTacInstr* instr = *it++;
	if(dynamic_cast<CTacLabel*>(instr) != NULL){
	  lbs.push_back(instr);
	}
      }
      labels[n] = lbs;
    }
  }
  return;
}

void function_inlining_block(CScope *m,
			     map<string, list<CTacInstr*>> &functions,
			     map<string, vector<CSymbol*>> &symbols,
			     map<string, list<CTacInstr*>> &labels) {
  CCodeBlock *cb = m->GetCodeBlock();

  list <CTacInstr*> &instrs = const_cast<list <CTacInstr*> &>(cb->GetInstr());

  list<CTacInstr*>::iterator it = instrs.begin();
  list<CTacInstr*> instrs_param;
  while (it != instrs.end()) {
    list<CTacInstr*>::iterator it_temp = it;
    CTacInstr* instr = *it++;
    if(instr->GetOperation() == opParam){
      instrs_param.push_back(instr);
    }
    else if(instr->GetOperation() == opCall){

      CTacName *n = dynamic_cast<CTacName*>(instr->GetSrc(1));
      assert(n != NULL);
      const CSymProc *proc = dynamic_cast<const CSymProc*>(n->GetSymbol());
      assert(proc != NULL);
      string name = proc->GetName();

      if(is_rte(name)){
	instrs_param.clear();
	continue;
      }

      CTacLabel* instr_ret = m->CreateLabel(name.c_str());

      list<CTacInstr*>instrs_func = functions[name];
      vector<CSymbol*>symbs_func = symbols[name];
      list<CTacInstr*> lbs_func = labels[name];

      map<const CSymbol*, const CSymbol*> new_symbs;
      map<CTacInstr*, CTacInstr*> new_lbs;
      map<int, const CSymbol*> params;

      {
	vector<CSymbol*>::iterator sit = symbs_func.begin();
	while (sit != symbs_func.end()) {
	  CSymbol *s = *sit++;
	  if(s->GetSymbolType() == stParam){
	    int index = dynamic_cast<CSymParam*>(s)->GetIndex();
	    const CSymbol *s_new = m->CreateTemp(s->GetDataType())->GetSymbol();
	    params[index] = s_new;
	    new_symbs[s] = s_new;
	  }
	  else if(s->GetSymbolType() == stLocal){
	    const CSymbol *s_new = m->CreateTemp(s->GetDataType())->GetSymbol();
	    new_symbs[s] = s_new;
	  }
	}
      }

      {
        list<CTacInstr*>::iterator lit = lbs_func.begin();
	while (lit != lbs_func.end()) {
	  CTacInstr *lb = *lit++;
	  CTacLabel* lb_new =
	    m->CreateLabel((dynamic_cast<CTacLabel*>(lb)->GetLabel().c_str()));
	  new_lbs[lb] = lb_new;
	}
      }

      {
	list<CTacInstr*>::iterator iit = instrs_func.begin();
	while (iit != instrs_func.end()) {
	  CTacInstr *instr_func = *iit++;

	  switch(instr_func->GetOperation()){
	  case opLabel:
	    {
	      assert(dynamic_cast<CTacLabel*>(instr_func) != NULL);
	      instrs.insert(it_temp, new_lbs[instr_func]);
	      break;
	    }
	  case opReturn:
	    {
	      if(instr->GetDest() != NULL){
		CTacAddr *src_new =
		  dynamic_cast<CTacAddr*>(convert_tac(new_symbs, new_lbs, instr_func->GetSrc(1)));
		instrs.insert(it_temp,
			      new CTacInstr(opAssign, instr->GetDest(), src_new, NULL));
	      }
	      instrs.insert(it_temp,
			    new CTacInstr(opGoto, instr_ret, NULL, NULL));
	      break;
	    }
	  default:
	    {
	      EOperation op = instr->GetOperation();
	      CTac *dst = instr->GetDest();
	      CTacAddr *src1 = instr->GetSrc(1);
	      CTacAddr *src2 = instr->GetSrc(2);

	      CTac *dst_new = convert_tac(new_symbs, new_lbs, dst);
	      CTacAddr *src1_new =
		dynamic_cast<CTacAddr*>(convert_tac(new_symbs, new_lbs, src1));
	      CTacAddr *src2_new =
		dynamic_cast<CTacAddr*>(convert_tac(new_symbs, new_lbs, src2));

	      CTacInstr *instr_new = new CTacInstr(op, dst_new, src1_new, src2_new);
	      instrs.insert(it_temp, instr_new);
	    }
	  }
	}
      }

      {
	list<CTacInstr*>::iterator iit = instrs_param.begin();
	while (iit != instrs_param.end()) {
	  CTacInstr *instr_param = *iit++;
	  int index = dynamic_cast<CTacConst*>(instr_param->GetDest())->GetValue();
	  instr_param->SetOperation(opAssign);
	  instr_param->SetDest(new CTacTemp(params[index]));
	}
      }

      {
	instrs.insert(it_temp, instr_ret);
	instrs.erase(it_temp);
      }

      instrs_param.clear();
    }
  }
}

void function_inlining_scope(CScope *m) {
  map<string, list<CTacInstr*>> functions;
  map<string, vector<CSymbol*>> symbols;
  map<string, list<CTacInstr*>> labels;
  get_functions(functions, symbols, labels, m);
  function_inlining_block(m, functions, symbols, labels);
  return;
}

void unused_function_scope(CScope *m) {
}
