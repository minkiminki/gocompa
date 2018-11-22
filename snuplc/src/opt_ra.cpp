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

  {
    Liveness *liveness = cbp->GetBlockTable()->GetLiveness();
    assert(liveness != NULL);
    map<const CSymbol*, list<const CSymbol*>> live_graph;
    list<CBasicBlock*>::const_iterator bit = cbp->GetBlockTable()->GetBlockList().begin();
    while (bit != cbp->GetBlockTable()->GetBlockList().end()) {
      CBasicBlock* blk = *bit++;

      {
  	list<const CSymbol*> &slist = (liveness->GetUses(1))[blk];
  	{
  	  list<const CSymbol*>::const_iterator sit = slist.begin();
  	  while (sit != slist.end()) {
  	    const CSymbol* s = *sit++;
  	    if(live_graph.find(s) == live_graph.end()){
  	      list<const CSymbol *> list_tmp;
  	      live_graph[s] = list_tmp;
  	    }
  	  }
  	}
  	{
  	  list<const CSymbol*>::const_iterator sit1 = slist.begin();
  	  while (sit1 != slist.end()) {
  	    const CSymbol* s1 = *sit1++;
  	    list<const CSymbol*> &lives1 = live_graph[s1];
  	    list<const CSymbol*>::const_iterator sit2 = sit1;
  	    while (sit2 != slist.end()) {
  	      const CSymbol* s2 = *sit2++;
  	      list<const CSymbol*> &lives2 = live_graph[s2];
  	      nodup_insert(lives1, s2);
  	      nodup_insert(lives2, s1);
  	    }
  	  }
  	}
      }

      {
  	list<const CSymbol*> &slist = (liveness->GetUses(2))[blk];
  	{
  	  list<const CSymbol*>::const_iterator sit = slist.begin();
  	  while (sit != slist.end()) {
  	    const CSymbol* s = *sit++;
  	    if(live_graph.find(s) == live_graph.end()){
  	      list<const CSymbol *> list_tmp;
  	      live_graph[s] = list_tmp;
  	      // live_graph[s] = new list<const CSymbol*>;
  	    }
  	  }
  	}
  	{
  	  list<const CSymbol*>::const_iterator sit1 = slist.begin();
  	  while (sit1 != slist.end()) {
  	    const CSymbol* s1 = *sit1++;
  	    list<const CSymbol*> &lives1 = live_graph[s1];
  	    list<const CSymbol*>::const_iterator sit2 = sit1;
  	    while (sit2 != slist.end()) {
  	      const CSymbol* s2 = *sit2++;
  	      list<const CSymbol*> &lives2 = live_graph[s2];
  	      nodup_insert(lives1, s2);
  	      nodup_insert(lives2, s1);
  	    }
  	  }
  	}
      }

      list<CTacInstr*>::const_iterator it = blk->GetInstrs().begin();
      while (it != blk->GetInstrs().end()) {
  	CTacInstr_prime* instr = dynamic_cast<CTacInstr_prime*>(*it++);
  	assert(instr != NULL);
  	{
  	  list<const CSymbol*> &slist = instr->GetLiveVars();
  	  {
  	    list<const CSymbol*>::const_iterator sit = slist.begin();
  	    while (sit != slist.end()) {
  	      const CSymbol* s = *sit++;
  	      if(live_graph.find(s) == live_graph.end()){
  		list<const CSymbol *> list_tmp;
  		live_graph[s] = list_tmp;
  	      }
  	    }
  	  }
  	  {
  	    list<const CSymbol*>::const_iterator sit1 = slist.begin();
  	    while (sit1 != slist.end()) {
  	      const CSymbol* s1 = *sit1++;
  	      list<const CSymbol*> &lives1 = live_graph[s1];
  	      list<const CSymbol*>::const_iterator sit2 = sit1;
  	      while (sit2 != slist.end()) {
  		const CSymbol* s2 = *sit2++;
  		list<const CSymbol*> &lives2 = live_graph[s2];
  		nodup_insert(lives1, s2);
  		nodup_insert(lives2, s1);
  	      }
  	    }
  	  }
  	}
      }
    }

    // cout << "liveness ---------------------------------------------" << endl;
    // map<const CSymbol*, list<const CSymbol*>>::iterator git = live_graph.begin();
    // while (git != live_graph.end()) {
    //   if(git->first->GetSymbolType() == stRegister) {
    // 	git++;
    // 	continue;
    //   }
    //   cout << (git->first->GetName()) << " -";
    //   list<const CSymbol*> &slist = git->second;
    //   list<const CSymbol*>::const_iterator sit = slist.begin();
    //   while (sit != slist.end()) {
    // 	cout << " " << (*sit++)->GetName();
    //   }
    //   cout << endl;
    //   git++;
    // }

  }

  {
    Liveness *liveness = cbp->GetBlockTable()->GetLiveness();
    assert(liveness != NULL);
    map<const CSymbol*, list<const CSymbol*>> assign_graph;
    list<CBasicBlock*>::const_iterator bit = cbp->GetBlockTable()->GetBlockList().begin();
    while (bit != cbp->GetBlockTable()->GetBlockList().end()) {
      CBasicBlock* blk = *bit++;

      list<CTacInstr*>::const_iterator it = blk->GetInstrs().begin();
      while (it != blk->GetInstrs().end()) {
  	CTacInstr_prime* instr = dynamic_cast<CTacInstr_prime*>(*it++);
  	assert(instr != NULL);
  	if(instr->GetOperation() != opAssign) continue;
  	CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
  	if(dest == NULL) continue;
  	if(dynamic_cast<CTacReference*>(dest) != NULL) continue;
  	CTacName* src = dynamic_cast<CTacName*>(instr->GetSrc(1));
        if(src == NULL) continue;
  	if(dynamic_cast<CTacReference*>(src) != NULL) continue;
  	const CSymbol* s_dest = dest->GetSymbol();
  	const CSymbol* s_src = src->GetSymbol();

  	if(assign_graph.find(s_dest) == assign_graph.end()){
  	  list<const CSymbol *> list_tmp;
  	  assign_graph[s_dest] = list_tmp;
  	}
  	if(assign_graph.find(s_src) == assign_graph.end()){
  	  list<const CSymbol *> list_tmp;
  	  assign_graph[s_src] = list_tmp;
  	}
  	list<const CSymbol*> &assign_dest = assign_graph[s_dest];
  	list<const CSymbol*> &assign_src = assign_graph[s_src];
  	nodup_insert(assign_dest, s_src);
  	nodup_insert(assign_src, s_dest);
      }

      it = blk->GetPhis().begin();
      while (it != blk->GetPhis().end()) {
  	CTacInstr_prime* instr = dynamic_cast<CTacInstr_prime*>(*it++);
  	assert(instr != NULL);

  	CTacName* dest = dynamic_cast<CTacName*>(instr->GetDest());
  	if(dest == NULL) continue;
  	if(dynamic_cast<CTacReference*>(dest) != NULL) continue;
  	const CSymbol* s_dest = dest->GetSymbol();
  	list<const CSymbol*> &assign_dest = assign_graph[s_dest];

  	{
  	  CTacName* src1 = dynamic_cast<CTacName*>(instr->GetSrc(1));
  	  if(src1 != NULL && dynamic_cast<CTacReference*>(src1) == NULL) {
  	    const CSymbol* s_src1 = src1->GetSymbol();
  	    if(assign_graph.find(s_src1) == assign_graph.end()){
  	      list<const CSymbol *> list_tmp;
  	      assign_graph[s_src1] = list_tmp;
  	    }
  	    list<const CSymbol*> &assign_src1 = assign_graph[s_src1];
  	    nodup_insert(assign_dest, s_src1);
  	    nodup_insert(assign_src1, s_dest);
  	  }
  	}

  	{
  	  CTacName* src2 = dynamic_cast<CTacName*>(instr->GetSrc(2));
  	  if(src2 != NULL && dynamic_cast<CTacReference*>(src2) == NULL) {
  	    const CSymbol* s_src2 = src2->GetSymbol();
  	    if(assign_graph.find(s_src2) == assign_graph.end()){
  	      list<const CSymbol *> list_tmp;
  	      assign_graph[s_src2] = list_tmp;
  	    }
  	    list<const CSymbol*> &assign_src2 = assign_graph[s_src2];
  	    nodup_insert(assign_dest, s_src2);
  	    nodup_insert(assign_src2, s_dest);
  	  }
  	}
      }
    }

    bool success = true;
    while(success){
      success = false;

      map<const CSymbol*, list<const CSymbol*>>::iterator git = assign_graph.begin();
      while (git != assign_graph.end()) {
  	const CSymbol* s1 = git->first;

  	list<const CSymbol*> &slist1 = git->second;
  	git++;
  	list<const CSymbol*>::const_iterator sit1 = slist1.begin();
  	while (sit1 != slist1.end()) {
  	  const CSymbol *s2 = *sit1++;

  	  list<const CSymbol*> &slist2 = assign_graph[s2];
  	  list<const CSymbol*>::const_iterator sit2 = slist2.begin();
  	  while (sit2 != slist2.end()) {
  	    const CSymbol *s3 = *sit2++;
  	    if(s1 != s3){
  	      list<const CSymbol*> &slist3 = assign_graph[s3];
  	      if(nodup_insert(slist1, s3) == 0){
  		success = true;
  		// cout << s1 << s2 << s3 << endl;
  	      }
  	      if(nodup_insert(slist3, s1) == 0){
  		success = true;
  		// cout << s1 << s2 << s3 << endl;
  	      }
  	    }
  	  }
  	}
      }

    }

    // cout << "assign ---------------------------------------------" << endl;
    // map<const CSymbol*, list<const CSymbol*>>::iterator git = assign_graph.begin();
    // while (git != assign_graph.end()) {
    //   if(git->first->GetSymbolType() == stRegister) {
    // 	git++;
    // 	continue;
    //   }
    //   cout << (git->first->GetName()) << " -";
    //   list<const CSymbol*> &slist = git->second;
    //   list<const CSymbol*>::const_iterator sit = slist.begin();
    //   while (sit != slist.end()) {
    // 	cout << " " << (*sit++)->GetName();
    //   }
    //   cout << endl;
    //   git++;
    // }


  }

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

	p->SetBaseRegister("");

	// p->SetBaseRegister("%rbp");
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

      s->SetBaseRegister("");
      // s->SetBaseRegister("%rbp");
      s->SetOffset(local_ofs);
    } else if (st == stParam) {
      if(param_num > 6){
	CSymParam *p = dynamic_cast<CSymParam*>(s);
	assert(p != NULL);

	p->SetBaseRegister("");
	// p->SetBaseRegister("%rbp");
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
