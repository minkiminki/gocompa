#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include <cassert>

#include "opt_bba.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Constant Propagation
int constant_propagation_block(CCodeBlock *cb) {
  int success = 0;
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  map<const CSymbol*, int> constants;

  list<CTacInstr*>::const_iterator it = cbp->GetInstr().begin();
  while (it != cbp->GetInstr().end()) {
    CTacInstr* instr = *it++;
    assert(instr != NULL);

    int c_src1 = 0;
    int c_src2 = 0;
    bool src1_constant = false;
    bool src2_constant = false;

    CTacName* src1 = dynamic_cast<CTacName*>(instr->GetSrc(1));
    if(src1 != NULL){
      if(dynamic_cast<CTacReference*>(src1) == NULL){
	const CSymbol* s_src1 = src1->GetSymbol();
        assert(s_src1 != NULL);
	map<const CSymbol*,int>::iterator _c_src1 = constants.find(s_src1);
	if(_c_src1 != constants.end()){
	  c_src1 = _c_src1->second;
	  src1_constant = true;
	  instr->SetSrc(0, new CTacConst(c_src1));
	}
      }
    }
    else{
      CTacConst* _src1 = dynamic_cast<CTacConst*>(instr->GetSrc(1));
      if(_src1 != NULL){
	c_src1 = _src1->GetValue();
	src1_constant= true;
      }
    }

    CTacName* src2 = dynamic_cast<CTacName*>(instr->GetSrc(2));
    if(src2 != NULL){
      if(dynamic_cast<CTacReference*>(src2) == NULL){
	const CSymbol* s_src2 = src2->GetSymbol();
	assert(s_src2 != NULL);
	map<const CSymbol*,int>::iterator _c_src2 = constants.find(s_src2);
	if(_c_src2 != constants.end()){
	  c_src2 = _c_src2->second;
	  src2_constant = true;
	  instr->SetSrc(1, new CTacConst(c_src2));
	}
      }
    }
    else{
      CTacConst* _src2 = dynamic_cast<CTacConst*>(instr->GetSrc(2));
      if(_src2 != NULL){
	c_src2 = _src2->GetValue();
	src2_constant= true;
      }
    }

    EOperation op = instr->GetOperation();

    switch(op){

    case opAdd:
    case opSub:
    case opMul:
    case opDiv:
    case opAnd:
    case opOr:
      {
	CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
	if(dest == NULL) continue;
	if(dynamic_cast<CTacReference*>(dest) != NULL) continue;

	const CSymbol* s_dest = dest->GetSymbol();
	if(s_dest == NULL) continue;
	if(s_dest->GetSymbolType() != stLocal) continue;

	if(src1_constant && src2_constant){
	  success = true;
	  instr->SetOperation(opNop);
	  instr->SetDest(NULL);
	  instr->SetSrc(0, NULL);
	  instr->SetSrc(1, NULL);
	  // TODO : remove symbol

	  // printf("herrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr\n");

	  switch(op){
	  case opAdd:
	    constants[s_dest] = c_src1 + c_src2; break;
	  case opSub:
	    //_P1;
	    constants[s_dest] = c_src1 - c_src2; break;
	  case opMul:
	    constants[s_dest] = c_src1 * c_src2; break;
	  case opDiv:
	    constants[s_dest] = c_src1 / c_src2; break;
	  case opAnd:
	    constants[s_dest] = c_src1 && c_src2; break;
	  case opOr:
	    constants[s_dest] = c_src1 || c_src2; break;
	  }
	}
      }
      break;

    case opNeg:
    case opPos:
    case opNot:
    case opAssign:
    case opMov:
    case opCast:
      {
	CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
	if(dest == NULL) continue;
	if(dynamic_cast<CTacReference*>(dest) != NULL) continue;

        const CSymbol* s_dest = dest->GetSymbol();
	if(s_dest == NULL) continue;
	if(s_dest->GetSymbolType() != stLocal) continue;

	if(src1_constant){
	  success = true;
	  instr->SetOperation(opNop);
	  instr->SetDest(NULL);
	  instr->SetSrc(0, NULL);
	  instr->SetSrc(1, NULL);
	  // TODO : remove symbol

	  // printf("herrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr\n");

	  switch(op){
	  case opNeg:
	    constants[s_dest] = - c_src1; break;
	  case opNot:
	    constants[s_dest] = ! c_src1; break;
	  case opPos:
	  case opAssign:
	  case opMov:
	  case opCast:
	    constants[s_dest] = c_src1; break;
	  }
	}
      }
      break;

    case opEqual:
    case opNotEqual:
    case opLessThan:
    case opLessEqual:
    case opBiggerThan:
    case opBiggerEqual:
      {
	CTacLabel_prime* dest = dynamic_cast<CTacLabel_prime*>(instr->GetDest());
	assert(dest != NULL);

	if(src1_constant && src2_constant){
	  success = true;
	  bool always = false;
	  // printf("herrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr\n");

	  switch(op){
	  case opEqual:
	    always = c_src1 == c_src2; break;
	  case opNotEqual:
	    always = c_src1 != c_src2; break;
	  case opLessThan:
	    always = c_src1 < c_src2; break;
	  case opLessEqual:
	    always = c_src1 <= c_src2; break;
	  case opBiggerThan:
	    always = c_src1 < c_src2; break;
	  case opBiggerEqual:
	    always = c_src1 <= c_src2; break;
	  }

	  if(always){
	    instr->SetOperation(opGoto);
	    instr->SetSrc(0, NULL);
	    instr->SetSrc(1, NULL);
	    // TODO : remove next, prev blks, phi
	  }
	  else{
	    instr->SetOperation(opNop);
	    instr->SetDest(NULL);
	    instr->SetSrc(0, NULL);
	    instr->SetSrc(1, NULL);
	    // TODO : remove next, prev blks, phi
	  }
	}
      }
    }
  }


  // map<const CSymbol*, int>::const_iterator sit = constants.begin();
  // while (sit != constants.end()) {
  //   cout << (sit->first) << " : " << (sit++->second) << endl;
  // }

  return success;
}

void constant_propagation_scope(CScope *m) {
  while(constant_propagation_block(m->GetCodeBlock()) > 0){
  }
  // constant_propagation_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    constant_propagation_scope(*sit++);
  }
  return;
}
