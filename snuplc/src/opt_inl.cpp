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

      // cout << s << endl;
      // if(s_new == NULL){
      // 	cout << "???" << endl;
      // }
      // else{
      // 	cout << s_new << endl;
      // }

    }

    if(s_deref->GetSymbolType() == stGlobal){
      s_deref_new = s_deref;
    }
    else if(s_deref->GetSymbolType() == stProcedure){
      s_deref_new = s_deref;
    }
    else{
      s_deref_new = new_symbs[s_deref];

      // cout << s_deref << endl;
      // if(s_deref_new == NULL){
      // 	cout << "???" << endl;
      // }
      // else{
      // 	cout << s_deref << endl;
      // }

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

      // cout << s << endl;
      // if(s_new == NULL){
      // 	cout << "???" << endl;
      // }
      // else{
      // 	cout << s << endl;
      // }

    }

    src_new = new CTacTemp(s_new);
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
    // cout << n << endl;
    {
    }
    functions[n] = sc->GetCodeBlock()->GetInstr();
    symbols[n] = sc->GetSymbolTable()->GetSymbols();
    {
      list<CTacInstr*> lbs;
      CCodeBlock *cb = sc->GetCodeBlock();
      list <CTacInstr*> &instrs = const_cast<list <CTacInstr*> &>(cb->GetInstr());
      list<CTacInstr*>::iterator it = instrs.begin();
      while (it != instrs.end()) {
	CTacInstr* instr = *it++;
	// cout << instr << endl;
	if(dynamic_cast<CTacLabel*>(instr) != NULL){
	  // cout << instr << endl;
	  // _P1;
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

  // cout << m->GetName() << endl;

  list <CTacInstr*> &instrs = const_cast<list <CTacInstr*> &>(cb->GetInstr());

  list<CTacInstr*>::iterator it = instrs.begin();
  list<list<CTacInstr*>::iterator> instrs_param;
  while (it != instrs.end()) {
    list<CTacInstr*>::iterator it_temp = it;
    CTacInstr* instr = *it++;
    if(instr->GetOperation() == opParam){
      // instrs_param.push_back(instr);
      instrs_param.push_back(it_temp);
    }
    else if(instr->GetOperation() == opCall){

      CTacName *n = dynamic_cast<CTacName*>(instr->GetSrc(1));
      assert(n != NULL);
      const CSymProc *proc = dynamic_cast<const CSymProc*>(n->GetSymbol());
      assert(proc != NULL);
      string name = proc->GetName();

      // cout << name << endl;
      // _P1;

      if(is_rte(name)){
	instrs_param.clear();
	continue;
      }

      list<CTacLabel*> instrs_ret;

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
	    int index = (dynamic_cast<CSymParam*>(s)->GetIndex())+1;
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


	  // cout << lb << endl;
	  // if(lb_new == NULL){
	  //   cout << "???" << endl;
	  // }
	  // else{
	  //   cout << lb_new << endl;
	  // }


	  new_lbs[lb] = lb_new;
	}
      }

      {
	list<CTacInstr*>::iterator iit = instrs_func.begin();
	while (iit != instrs_func.end()) {
	  CTacInstr *instr_func = *iit++;
	  // cout << instr_func << endl;


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

	      CTacLabel* instr_ret = m->CreateLabel(name.c_str());
	      instrs_ret.push_back(instr_ret);
	      instrs.insert(it_temp,
			    new CTacInstr(opGoto, instr_ret, NULL, NULL));
	      break;
	    }
	  default:
	    {

	      EOperation op = instr_func->GetOperation();
	      CTac *dst = instr_func->GetDest();
	      CTacAddr *src1 = instr_func->GetSrc(1);
	      CTacAddr *src2 = instr_func->GetSrc(2);

	      // cout << op << endl;

	      // _P1;
	      // cout << instr_func << endl;


	      CTac *dst_new = convert_tac(new_symbs, new_lbs, dst);
	      // _P2;



	      // if(dst == NULL){
	      // 	cout << "???" << endl;
	      // }
	      // else{
	      // 	cout << dst << endl;
	      // }

	      // if(dst_new == NULL){
	      // 	cout << "???" << endl;
	      // }
	      // else{
	      // 	cout << dst_new << endl;
	      // }


	      CTacAddr *src1_new =
		dynamic_cast<CTacAddr*>(convert_tac(new_symbs, new_lbs, src1));


	      // if(src1 == NULL){
	      // 	cout << "???" << endl;
	      // }
	      // else{
	      // 	cout << src1 << endl;
	      // }

	      // if(src1_new == NULL){
	      // 	cout << "???" << endl;
	      // }
	      // else{
	      // 	cout << src1_new << endl;
	      // }

	      CTacAddr *src2_new =
		dynamic_cast<CTacAddr*>(convert_tac(new_symbs, new_lbs, src2));

	      // if(src2 == NULL){
	      // 	cout << "???" << endl;
	      // }
	      // else{
	      // 	cout << src2 << endl;
	      // }

	      // if(src2_new == NULL){
	      // 	cout << "???" << endl;
	      // }
	      // else{
	      // 	cout << src2_new << endl;
	      // }


	  //       _P2;

	  // cout << instr_func << endl;


	      CTacInstr *instr_new = new CTacInstr(op, dst_new, src1_new, src2_new);
	      	  // _P1;

	      instrs.insert(it_temp, instr_new); // OK

	    }
	  }


	}
      }




      {
	list<list<CTacInstr*>::iterator>::iterator iit = instrs_param.begin();
	while (iit != instrs_param.end()) {
	  list<CTacInstr*>::iterator instr_param = *iit++;
	  int index = dynamic_cast<CTacConst*>((*instr_param)->GetDest())->GetValue();


	  // cout << index << endl;

	  CTacInstr *instr_param_new = new CTacInstr(opAssign, new CTacTemp(params[index]),
						     (*instr_param)->GetSrc(1), NULL);

	  *instr_param = instr_param_new;
	  // *iit = instr_param_new;

	  // instr_param->SetOperation(opAssign);
	  // instr_param->SetDest(new CTacTemp(params[index]));

	}
      }



      {
	list<CTacLabel*>::iterator iit = instrs_ret.begin();
	while (iit != instrs_ret.end()) {
	  instrs.insert(it_temp, *iit++); // OK
	}
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

  // vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  // while (sit != m->GetSubscopes().end()) {
  //   function_inlining_block(*sit++, functions, symbols, labels);
  // }


  return;
}

void unused_function_scope(CScope *m) {
}
