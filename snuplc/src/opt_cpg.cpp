#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include <cassert>

#include "opt_bba.h"
#include "opt_cpg.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Constant Propagation
int CalcDims(const CSymbol *s){
  // cout << s << endl;
  const CPointerType* tp = dynamic_cast<const CPointerType*>(s->GetDataType());
  assert(tp != NULL);

  const CArrayType* tbp = dynamic_cast<const CArrayType*>((dynamic_cast<const CPointerType*>(tp))->GetBaseType());
  assert(tbp != NULL);

  return tbp->GetNDim();
}

int CalcDOFS(const CSymbol *s){
  int d = CalcDims(s);
  if(d < 0){
    return -1;
  }
  else if(d%2 == 0){
    return (4*d + 8);
  }
  else{
    return (4*d + 4);
  }
}

int CalcDIM(const CSymbol *s, int index){
  const CPointerType* tp = dynamic_cast<const CPointerType*>(s->GetDataType());
  if(tp == NULL) return 0;

  const CArrayType* tbp = dynamic_cast<const CArrayType*>((dynamic_cast<const CPointerType*>(tp))->GetBaseType());

  for(; index > 1; index--){
    if(tbp == NULL) return 0;
    tbp = dynamic_cast<const CArrayType*>(tbp->GetInnerType());
  }

  return tbp->GetNElem();
}


// int CalcDOFS(const CSymbol *s){
//   if(s == NULL) return -1;
//   const CType* tp = s->GetDataType();
//   assert(tp != NULL);
//   assert(tp->IsPointer());

//   const CArrayType* tbp = dynamic_cast<const CArrayType*>((dynamic_cast<const CPointerType*>(tp))->GetBaseType());
//   Get
//   assert(tbp != NULL);

//   const CCArrayType

//   GetBaseType

//   while(true){
//     if(tp->IsNull) return -1;
//     if(tp->

//     const CPointerType* = dynamic_cast<const CPointerType*>(tp);


//     const CPointerType* = dynamic_cast<const CPointerType*>(tp);
//     if(tp



//   }

//   const CPointerType CType* s->GetDataType();
//   s

//   assert(s != NULL)

// }


// int CalcDOFS(const CSymbol *s){
//   if(s == NULL) return -1;
//   const CType* tp = s->GetDataType();
//   assert(tp != NULL);
//   assert(tp->IsPointer());

//   const CArrayType* tbp = dynamic_cast<const CArrayType*>((dynamic_cast<const CPointerType*>(tp))->GetBaseType());
//   Get
//   assert(tbp != NULL);

//   const CCArrayType

//   GetBaseType

//   while(true){
//     if(tp->IsNull) return -1;
//     if(tp->

//     const CPointerType* = dynamic_cast<const CPointerType*>(tp);


//     const CPointerType* = dynamic_cast<const CPointerType*>(tp);
//     if(tp



//   }

//   const CPointerType CType* s->GetDataType();
//   s

//   assert(s != NULL)

// }


int constant_propagation_block(CCodeBlock *cb) {
  int success = 0;
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  map<const CSymbol*, int> constants;

  list<CBasicBlock*>::const_iterator bit = cbp->GetBlockTable()->GetBlockList().begin();
  while (bit != cbp->GetBlockTable()->GetBlockList().end()) {
    CBasicBlock* blk = *bit++;

    list<CTacInstr*>::const_iterator it = blk->GetPhis().begin();
    while (it != blk->GetPhis().end()) {
      list<CTacInstr*>::const_iterator it_temp = it++;
      CTacInstr* instr = *it_temp;
      assert(instr != NULL);

      int c_src1 = 0;
      int c_src2 = 0;
      bool src1_constant = false;
      bool src2_constant = false;

      {
	CTacName* src1 = dynamic_cast<CTacName*>(instr->GetSrc(1));
	if(src1 != NULL){
	  assert(dynamic_cast<CTacReference*>(src1) == NULL);
	  const CSymbol* s_src1 = src1->GetSymbol();
	  assert(s_src1 != NULL);
	  map<const CSymbol*,int>::iterator _c_src1 = constants.find(s_src1);
	  if(_c_src1 != constants.end()){
	    c_src1 = _c_src1->second;
	    src1_constant = true;
	    instr->SetSrc(0, new CTacConst(c_src1));
	  }
	}
	else{
	  CTacConst* _src1 = dynamic_cast<CTacConst*>(instr->GetSrc(1));
	  assert(_src1 != NULL);
	  c_src1 = _src1->GetValue();
	  src1_constant= true;
	}
      }

      {
	CTacName* src2 = dynamic_cast<CTacName*>(instr->GetSrc(2));
	if(src2 != NULL){
	  assert(dynamic_cast<CTacReference*>(src2) == NULL);
	  const CSymbol* s_src2 = src2->GetSymbol();
	  assert(s_src2 != NULL);
	  map<const CSymbol*,int>::iterator _c_src2 = constants.find(s_src2);
	  if(_c_src2 != constants.end()){
	    c_src2 = _c_src2->second;
	    src2_constant = true;
	    instr->SetSrc(1, new CTacConst(c_src2));
	  }
	}
	else{
	  CTacConst* _src2 = dynamic_cast<CTacConst*>(instr->GetSrc(2));
	  assert(_src2 != NULL);
	  c_src2 = _src2->GetValue();
	  src2_constant= true;
	}
      }

      if(src1_constant && src2_constant){
	if(c_src1 == c_src2){
	  // cout << instr << endl;
	  // _P1;
	  CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
	  assert(dest != NULL);
	  assert(dynamic_cast<CTacReference*>(dest) == NULL);
	  const CSymbol* s_dest = dest->GetSymbol();
	  assert(s_dest != NULL);
	  constants[s_dest] = c_src1;
	  (blk->GetPhis()).erase(it_temp);
	}
      }
    }


    it = blk->GetInstrs().begin();
    // list<CTacInstr*>::const_iterator it = blk->GetInstrs().begin();
    while (it != blk->GetInstrs().end()) {
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
      case opGetParam:
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
	    case opGetParam:
	    case opCast:
	      constants[s_dest] = c_src1; break;
	    }
	  }
	}
	break;

      case opDIM:
	{
	  CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
	  if(dest == NULL) continue;
	  if(dynamic_cast<CTacReference*>(dest) != NULL) continue;

	  const CSymbol* s_dest = dest->GetSymbol();
	  if(s_dest == NULL) continue;
	  if(s_dest->GetSymbolType() != stLocal) continue;

	  const CSymbol* s_src1 = src1->GetSymbol();


	  if(src2_constant){
	    int d = CalcDIM(s_src1, c_src2);
	    if(d != -1){
	      success = true;
	      instr->SetOperation(opNop);
	      instr->SetDest(NULL);
	      instr->SetSrc(0, NULL);
	      instr->SetSrc(1, NULL);
	      constants[s_dest] = d;
	    }
	  }
	}
	break;

      case opDOFS:
	{
	  CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
	  if(dest == NULL) continue;
	  if(dynamic_cast<CTacReference*>(dest) != NULL) continue;

	  const CSymbol* s_dest = dest->GetSymbol();
	  if(s_dest == NULL) continue;
	  if(s_dest->GetSymbolType() != stLocal) continue;

	  const CSymbol* s_src1 = src1->GetSymbol();

	  int d = CalcDOFS(s_src1);
	  if(d != -1){
	    success = true;
	    instr->SetOperation(opNop);
	    instr->SetDest(NULL);
	    instr->SetSrc(0, NULL);
	    instr->SetSrc(1, NULL);
	    constants[s_dest] = d;
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
	      always = c_src1 > c_src2; break;
	    case opBiggerEqual:
	      always = c_src1 >= c_src2; break;
	    }

	    if(always){
	      instr->SetOperation(opGoto);
	      instr->SetSrc(0, NULL);
	      instr->SetSrc(1, NULL);

	      CBasicBlock* blk_next = dynamic_cast<CTacInstr_prime*>
				(instr->GetDest())->GetFromBlock();
	      list<CBasicBlock*>::const_iterator bit2 = blk->GetNextBlks().begin();
	      while (bit2 != blk->GetNextBlks().end()) {
		CBasicBlock* blk_next2 = *bit2++;
		if(blk_next2 != blk_next){
		  assert(erase_success(blk->GetNextBlks(), blk_next2) >= 0);
		  assert(erase_success(blk_next2->GetPrevBlks(), blk) >= 0);
		  break;
		}
	      }
	      // TODO : remove next, prev blks, phi
	    }
	    else{

	      CBasicBlock* blk_next = dynamic_cast<CTacInstr_prime*>
		(instr->GetDest())->GetFromBlock();
	      assert(erase_success(blk->GetNextBlks(), blk_next) >= 0);
	      assert(erase_success(blk_next->GetPrevBlks(), blk) >= 0);

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
  }


  // map<const CSymbol*, int>::const_iterator sit = constants.begin();
  // while (sit != constants.end()) {
  //   cout << (sit->first) << " : " << (sit++->second) << endl;
  // }

  return success;
}

void constant_propagation_scope(CScope *m) {
  while(constant_propagation_block(m->GetCodeBlock()) > 0){
    remove_unreachable_block(m->GetCodeBlock());
  }
  // constant_propagation_block(m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    constant_propagation_scope(*sit++);
  }
  return;
}
