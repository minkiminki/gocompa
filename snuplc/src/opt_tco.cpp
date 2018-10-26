#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "opt_tco.h"
using namespace std;


// ********************************************************************** /
// ********************************************************************** /
// Tail Call optimization
void tail_call_optimization_block(int arch, CCodeBlock *cb) {
  CCodeBlock_prime *cbp = dynamic_cast<CCodeBlock_prime*>(cb);
  assert(cbp != NULL);

  CBlockTable *cbt = cbp->GetBlockTable();
  assert(cbt != NULL);

  CTacName *n;

  list<CBasicBlock*>::const_iterator it = (cbt->GetFinBlocks()).begin();
  while (it != (cbt->GetFinBlocks()).end()){
    list<CTacInstr*> instrs = (*it++)->GetInstrs();
    list<CTacInstr*>::const_reverse_iterator iit = instrs.rbegin();

    while((iit != instrs.rend())){
      assert(*iit != NULL);
      if((*iit)->GetOperation() == opNop){
	iit++;
	continue;
      }
      else{
	break;
      }
    }

    CTacInstr* instr1;
    CTacInstr* instr0;

    if(iit != instrs.rend()){
      instr0 = *iit++;
      if(instr0->GetOperation() == opReturn){
	while(iit != instrs.rend()){
	  assert(*iit != NULL);
	  if((*iit)->GetOperation() == opNop){
	    iit++;
	    continue;
	  }
	  else{
	    break;
	  }
	}

	if(iit != instrs.rend()){
	   instr1 = *iit++;
	   if(instr1->GetOperation() == opCall){

	     if((instr1->GetDest()) == (instr0->GetSrc(1))){
	     }
	     else{
	       continue;
	     }
	   }
	   else{
	     continue;
	   }
	 }
	 else{
	   continue;
	 }
      }
      else if(instr0->GetOperation() == opCall){
	instr1 = instr0;
      }
      else{
	continue;
      }
    }
    else{
      continue;
    }

    CTacName *n = dynamic_cast<CTacName*>(instr1->GetSrc(1));
    assert(n != NULL);
    const CSymProc *proc = dynamic_cast<const CSymProc*>(n->GetSymbol());
    assert(proc != NULL);

    int i = proc->GetNParams();
    if((i > 6) || (arch == 32 && i > 0)){
      continue;
    }

    bool exitloop = false;
    for(; i >= 1; i--){
      if(iit == instrs.rend()){
	exitloop = true;
	break;
      }
      instr0 = *iit++;
      assert(instr0 != NULL);

      if(instr0->GetOperation() == opParam){
        n = dynamic_cast<CTacName*>(instr0->GetSrc(1));
	if(n == NULL){
	  assert(dynamic_cast<CTacConst*>(instr0->GetSrc(1))!= NULL);
	  continue;
	}
	assert(n != NULL);
	const CSymbol *s = n->GetSymbol();
	assert(s != NULL);
        if((s->GetDataType()->IsPointer()) || (s->GetDataType()->IsArray())){
	  if((s->GetSymbolType() != stGlobal) && (s->GetSymbolType() != stParam)){

	    if(iit != instrs.rend()){
	      instr0 = *iit;
	      assert(instr0 != NULL);
	      if(instr0->GetOperation() == opAddress){
		if(n == dynamic_cast<CTacName*>(instr0->GetDest())){
		  n = dynamic_cast<CTacName*>(instr0->GetSrc(1));
		  assert(n != NULL);
		  const CSymbol *s = n->GetSymbol();
		  assert(s != NULL);
		  if((s->GetSymbolType() == stGlobal) || (s->GetSymbolType() == stParam)){
		    i--;
		    continue;
		  }
		}
	      }
	    }
	    exitloop = true;
	    break;
	  }
	}
	i--;
      }
    }

    if(exitloop){
      break;
    }
    else{
      instr1->SetOperation(opTailCall);
    }
  }
}

void tail_call_optimization_scope(int arch, CScope *m){
  tail_call_optimization_block(arch, m->GetCodeBlock());

  vector<CScope*>::const_iterator sit =m->GetSubscopes().begin();
  while (sit != m->GetSubscopes().end()) {
    tail_call_optimization_scope(arch, *sit++);
  }
  return;
}
