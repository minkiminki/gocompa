#include <iomanip>
#include <cassert>
#include <map>
#include <algorithm>
#include <list>

#include "ir2.h"
using namespace std;



template<typename T>
int list_join(list<T>& l1, list<T>& l2){
  int success = 0;
  typename list<T>::iterator it = l1.begin();
  while(it != l1.end()){
    typename list<T>::iterator it_before = it++;
    T key = *(it_before);
    typename list<T>::iterator findit = find(l2.begin(), l2.end(), key);
    if(findit != l2.end()){
      l1.erase(it_before);
      success = 1;
    }
  }
  return success;
}


//------------------------------------------------------------------------------
// CTacInstr
//

CTacInstr_prime::CTacInstr_prime(CTacInstr *instr)
  : CTacInstr(opNop, instr->GetDest(), instr->GetSrc(1), instr->GetSrc(2)), _block(NULL)
{
  SetOperation(instr->GetOperation());
}

CTacInstr_prime::~CTacInstr_prime(void)
{
  if (IsBranch()) {
    CTacLabel_prime *lbl = dynamic_cast<CTacLabel_prime*>(_dst);
    assert(lbl != NULL);
    lbl->AddReference(-1);
  }
}

ostream& CTacInstr_prime::print(ostream &out, int indent) const
{
  CBasicBlock *block = GetFromBlock();
  if(block != NULL) {
    // out << "  [" << (block->GetBlockNum()) << "]";
    out << "  [";
    out.flags(ios::right);
    out.width(3);
    out << (block->GetBlockNum());
    out << "]";
  }
  else{
    out << "  [---]";
  }

  string ind(indent, ' ');

  out << ind << right << dec << setw(3) << _id << ": ";

  if (_name == "") {
    bool relop = IsRelOp(GetOperation());

    out << "    " << left << setw(6);
    if (relop) out << "if"; else out << _op;
    out << " ";
    CTacAddr *adr = dynamic_cast<CTacAddr*>(_dst);
    if (adr != NULL) out << _dst << " <- ";
    if (_src1 != NULL) out << _src1;
    if (_src2 != NULL) {
      if (relop) out << " " << _op; else out << ",";
      out << " " << _src2;
    }
    CTacInstr *target = dynamic_cast<CTacInstr*>(_dst);
    if (target != NULL) {
      if (relop) out << " goto ";

      CTacLabel_prime *l = dynamic_cast<CTacLabel_prime*>(target);
      if (l != NULL) out << l->GetLabel();
      else out << target->GetId();
    }
  } else {
    out << "[CTacInstr: '" << _name << "']";
  }

  return out;
}


//------------------------------------------------------------------------------
// CTacLabel
//

CTacLabel_prime::CTacLabel_prime(CTacInstr *instr, const string label, int refcnt)
  : CTacInstr_prime(instr), _label(label), _refcnt(refcnt)
{
}

CTacLabel_prime::~CTacLabel_prime(void)
{
}

const string CTacLabel_prime::GetLabel(void) const
{
  return _label;
}

int CTacLabel_prime::AddReference(int ofs)
{
  _refcnt += ofs;
  return _refcnt;
}

int CTacLabel_prime::GetRefCnt(void) const
{
  return _refcnt;
}

ostream& CTacLabel_prime::print(ostream &out, int indent) const
{
  if (true || GetRefCnt() > 0) {

    CBasicBlock *block = GetFromBlock();
    if(block != NULL) {
      // out << "  [" << (block->GetBlockNum()) << "]";
      out << "  [";
      out.flags(ios::right);
      out.width(3);
      out << (block->GetBlockNum());
      out << "]";
    }
    else{
      out << "  [---]";
    }

    string ind(indent, ' ');

    out << ind << right << dec << setw(3) << _id << ": "
        << left << _label << ":"
        // << "  (refcnt: " << _refcnt << ")"
        ;
  }

  return out;
}


//------------------------------------------------------------------------------
// CCodeBlock
//
CCodeBlock_prime::CCodeBlock_prime(CCodeBlock *cblock)
  : CCodeBlock(cblock->GetOwner())
{
  assert(_owner != NULL);

  map<CTac*, CTacInstr*> labelmap;

  list<CTacInstr*>::const_iterator it = (cblock->GetInstr()).begin();
  while (it != (cblock->GetInstr()).end()) {
    CTacInstr *instr = *it++;
    CTacLabel *lbl = dynamic_cast<CTacLabel*>(instr);
    CTacInstr_prime *newinstr;

    if (lbl != NULL) {
      newinstr = new CTacLabel_prime(lbl, lbl->GetLabel(), lbl->GetRefCnt());
      labelmap[lbl] = newinstr;
      //      labelmap.insert(std::pair<CTacInstr*, CTacLabel_prime*>(lbl, newinstr));
    }
    else {
      newinstr= new CTacInstr_prime(instr);
    }
    AddInstr(newinstr);
  }

  it = _ops.begin();
  while (it != _ops.end()) {
    CTacInstr *instr = *it++;
    map<CTac*, CTacInstr*>::iterator t = labelmap.find(instr->GetDest());
    if (t != labelmap.end())
      instr->SetDest(t->second);
  }

  _blktab = new CBlockTable();
  _size = 0;
	_param_num = 0;

}

CCodeBlock_prime::~CCodeBlock_prime(void)
{
  //TODO same for normal CCodeBlock
}

ostream& CCodeBlock_prime::print(ostream &out, int indent) const
{
  CCodeBlock::print(out, indent);

  CBlockTable* bt = GetBlockTable();
  if(bt == NULL){
    out << "[ no block table] ";
  }
  else{
    out << GetBlockTable();
  }

  return out;
}

CBasicBlock::CBasicBlock()
{
}

CBasicBlock::~CBasicBlock(void)
{
}

list<CBasicBlock*>& CBasicBlock::GetPrevBlks(void)
{
  return _prevblks;
}

list<CBasicBlock*>& CBasicBlock::GetNextBlks(void)
{
  return _nextblks;
}

list<CBasicBlock*>& CBasicBlock::GetPreDoms(void)
{
  return _predoms;
}

list<CBasicBlock*>& CBasicBlock::GetDoms(void)
{
  return _doms;
}

void CBasicBlock::AddInstr(CTacInstr *instr)
{
  assert(instr != NULL);
  _instrs.push_back(instr);
  return;
}

void CBasicBlock::AddPrevBlks(CBasicBlock *prev)
{
  nodup_insert(_prevblks, prev);
  // //  assert(prev != NULL);
  // _prevblks.push_back(prev);
  return;
}

void CBasicBlock::AddNextBlks(CBasicBlock *next)
{
  nodup_insert(_nextblks, next);
  //  assert(next != NULL);
  // _nextblks.push_back(next);
  return;
}


void CBasicBlock::AddPreDoms(CBasicBlock *prev)
{
  nodup_insert(_predoms, prev);
  // //  assert(prev != NULL);
  // _prevblks.push_back(prev);
  return;
}

void CBasicBlock::AddDoms(CBasicBlock *next)
{
  nodup_insert(_doms, next);
  //  assert(next != NULL);
  // _nextblks.push_back(next);
  return;
}

void CBasicBlock::SetDoms(list<CBasicBlock*> doms)
{
  _doms = doms;
}


int CBasicBlock::DomsJoin(list<CBasicBlock*>& doms)
{
  return list_join(_doms, doms);
}

list<CTacInstr*>& CBasicBlock::GetInstrs(void)
{
  return _instrs;
}

void CBasicBlock::SetBlockNum(int blocknum)
{
  _blocknum = blocknum;
}

int CBasicBlock::GetBlockNum(void) const
{
  return _blocknum;
}

ostream& CBasicBlock::print(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << "(" << GetBlockNum() << " -";

  list<CBasicBlock*>::const_iterator it = _prevblks.begin();
  while (it != _prevblks.end()){
    CBasicBlock* blk = *it++;
    if(blk == NULL){
      out << " IN";
    }
    else{
      out << " " << (blk->GetBlockNum());
    }
  }

  out <<" /";

  it = _nextblks.begin();
  while (it != _nextblks.end()){
    CBasicBlock* blk = *it++;
    if(blk == NULL){
      out << " OUT";
    }
    else{
      out << " " << (blk->GetBlockNum());
    }
  }
  // out << ")";
  out << " |";
  it = _doms.begin();
  while (it != _doms.end()){
    CBasicBlock* blk = *it++;
    if(blk == NULL){
      out << " OUT";
    }
    else{
      out << " " << (blk->GetBlockNum());
    }
  }
  out << ")";

  return out;
}

CBlockTable::CBlockTable(void)
  : maxblock(0), _initblock(NULL)
{
}

CBlockTable::~CBlockTable(void)
{
}

list<CBasicBlock*>& CBlockTable::GetBlockList(void)
{
  return _blocklist;
}

int CBlockTable::AddBlock(CBasicBlock *block)
{
  assert(block != NULL);
  block->SetBlockNum(++maxblock);
  _blocklist.push_back(block);
  return maxblock;
}

void CBlockTable::CombineBlock(CBasicBlock* blk, CBasicBlock* blk_next)
{
  assert(blk != NULL);
  assert(blk_next != NULL);

  /* 1 - remove from list */
  assert(erase_success(_blocklist, blk_next) >= 0);
  assert(erase_success(blk->GetNextBlks(), blk_next) >= 0);

  /* 2 - remove from nnextblock */
  list<CBasicBlock*>::iterator it = (blk_next->GetNextBlks()).begin();
  while (it != (blk_next->GetNextBlks()).end()) {
    CBasicBlock* blk_nnext = *it++;
    blk->AddNextBlks(blk_nnext);

    if(blk_nnext == NULL){
      assert(erase_success(_finblocks, blk_next) >= 0);
      nodup_insert(_finblocks, blk);
    }
    else{
      assert(erase_success(blk_nnext->GetPrevBlks(), blk_next) >= 0);
      blk_nnext->AddPrevBlks(blk);
    }
  }

  /* 3 - erase instrs */
  list<CTacInstr*>::iterator iit = blk_next->GetInstrs().begin();
  while (iit != (blk_next->GetInstrs()).end()) {
    CTacInstr* instr = *iit++;
    assert(instr != NULL);
    CTacInstr_prime *instrp = dynamic_cast<CTacInstr_prime*>(instr);
    assert(instrp != NULL);
    instrp->SetFromBlock(blk);
  }
  (blk->GetInstrs()).insert((blk->GetInstrs()).end(),
			    (blk_next->GetInstrs()).begin(),
			    (blk_next->GetInstrs()).end());
}

void CBlockTable::RemoveBlock(CBasicBlock *blk)
{
  assert(blk != NULL);

  /* 1 - remove from list */
  assert(erase_success(_blocklist, blk) >= 0);

  /* 2 - remove from nextblock */
  list<CBasicBlock*>::iterator it = (blk->GetNextBlks()).begin();
  while (it != (blk->GetNextBlks()).end()) {
    CBasicBlock* blk_next = *it++;
    if(blk_next == NULL){
      assert(erase_success(_finblocks, blk) >= 0);
    }
    else{
      assert(erase_success(blk_next->GetPrevBlks(), blk) >= 0);
    }
  }

  /* 3 - erase instrs */
  list<CTacInstr*>::iterator iit = blk->GetInstrs().begin();
  while (iit != (blk->GetInstrs()).end()) {
    CTacInstr* instr = *iit++;
    assert(instr != NULL);
    CTacInstr_prime *instrp = dynamic_cast<CTacInstr_prime*>(instr);
    assert(instrp != NULL);
    instrp->SetFromBlock(NULL);
    // assert(cb->RemoveInstr(instr) >=0);
  }

  /* 4 - delete it */

}

void CBlockTable::SetInitBlock(CBasicBlock* initblock)
{
  _initblock = initblock;
}

void CBlockTable::AddFinBlock(CBasicBlock* finblock)
{
  nodup_insert(_finblocks, finblock);
  return;
}

list<CBasicBlock*>& CBlockTable::GetFinPreDoms(void)
{
  return _finpredoms;
}

void CBlockTable::AddFinPreDom(CBasicBlock* block)
{
  nodup_insert(_finpredoms, block);
}

CBasicBlock* CBlockTable::GetInitBlock(void) const
{
  return _initblock;
}

list<CBasicBlock*>& CBlockTable::GetFinBlocks(void)
{
  return _finblocks;
}

void CBlockTable::BlockRenumber(void)
{
  maxblock = 0;
  list<CBasicBlock*>::const_iterator it = _blocklist.begin();
  while(it != _blocklist.end()){
    CBasicBlock* cb = *it++;
    assert(cb != NULL);
    cb->SetBlockNum(++maxblock);
  }
}

CBasicBlock* CTacInstr_prime::GetFromBlock(void) const
{
  return _block;
}

void CTacInstr_prime::SetFromBlock(CBasicBlock* block)
{
  _block = block;
}

CBlockTable* CCodeBlock_prime::GetBlockTable() const
{
  return _blktab;
}

int CCodeBlock_prime::GetStackSize() const
{
  return _size;
}

void CCodeBlock_prime::SetStackSize(int size)
{
  _size = size;
}

int CCodeBlock_prime::GetParamNum() const
{
	return _param_num;
}

void CCodeBlock_prime::SetParamNum(int param_num)
{
	_param_num = param_num;
}


EOperation OpToggle(EOperation op)
{
  switch(op){
  case opEqual : return opNotEqual;
  case opNotEqual : return opEqual;
  case opLessThan : return opBiggerEqual;
  case opLessEqual : return opBiggerThan;
  case opBiggerThan : return opLessEqual;
  case opBiggerEqual : return opLessThan;
  default : assert(false);
  }
}

void CCodeBlock_prime::SplitIf(CTacInstr_prime* instr)
{
  assert(instr != NULL);
  assert(IsRelOp(instr->GetOperation()));
  instr->SetOperation(OpToggle(instr->GetOperation()));

  CTacLabel *_lb = _owner->CreateLabel(NULL);
  CTacLabel_prime *lb = new CTacLabel_prime(_lb, _lb->GetLabel(),_lb->GetRefCnt());
  CTacInstr_prime *go = new CTacInstr_prime(new CTacInstr(opNop, NULL));
  go->SetOperation(opGoto);

  CTacInstr_prime *lbl = dynamic_cast<CTacInstr_prime*>(instr->GetDest());
  assert(lbl != NULL);

  go->SetDest(lbl);
  instr->SetDest(lb);

  CBasicBlock* bb_prev = instr->GetFromBlock();
  CBasicBlock* bb = lbl->GetFromBlock();
  CBasicBlock *bb_new = new CBasicBlock();
  GetBlockTable()->AddBlock(bb_new);

  list<CTacInstr*>::iterator it = find(_ops.begin(), _ops.end(), instr);

  assert(it != _ops.end());


  it++;
  CTacInstr_prime *instr_next = dynamic_cast<CTacInstr_prime*>(*it);

  CBasicBlock* bb_next = instr_next->GetFromBlock();

  assert(erase_success(bb_prev->GetNextBlks(), bb) >= 0);
  assert(erase_success(bb->GetPrevBlks(), bb_prev) >= 0);

  bb_prev->AddNextBlks(bb_new);
  bb_new->AddPrevBlks(bb_prev);

  assert(bb != NULL);

  bb->AddPrevBlks(bb_new);

  bb_new->AddNextBlks(bb);



  lb->SetFromBlock(bb_next);
  go->SetFromBlock(bb_new);

  bb_new->AddInstr(go);
  (bb_next->GetInstrs()).push_front(lb);

  _ops.insert(it, go);
  _ops.insert(it, lb);
}

void CCodeBlock_prime::SplitElse(CBasicBlock* bb_prev, CBasicBlock* bb)
{
  printf("CHECK IT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1!!!\n");
  printf("CHECK IT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1!!!\n");
  printf("CHECK IT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1!!!\n");
  printf("CHECK IT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1!!!\n");
  printf("CHECK IT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1!!!\n");
  assert(bb!=NULL);
  CTacInstr *instr = *(bb->GetInstrs().begin());
  assert(instr != NULL);
  CTacLabel_prime* lb = dynamic_cast<CTacLabel_prime*>(instr);
  assert(lb!=NULL);
  // assert(lb->GetSymbol()!=NULL);

  CTacInstr *_instr_new = new CTacInstr(opNop, instr, NULL, NULL);
  CTacInstr_prime *instr_new = new CTacInstr_prime(_instr_new);
  instr_new->SetOperation(opGoto);

  assert(erase_success(bb_prev->GetNextBlks(), bb) >= 0);
  assert(erase_success(bb->GetPrevBlks(), bb_prev) >= 0);

  CBasicBlock *bb_new = new CBasicBlock();
  GetBlockTable()->AddBlock(bb_new);
  bb_prev->AddNextBlks(bb_new);
  bb_new->AddPrevBlks(bb_prev);
  bb->AddPrevBlks(bb_new);
  bb_new->AddNextBlks(bb);

  bb_new->AddInstr(instr_new);
  list<CTacInstr*>::iterator it = find(_ops.begin(), _ops.end(), instr);
  assert(it != _ops.end());
  _ops.insert(it, instr_new);
  instr_new->SetFromBlock(bb_new);
}

list<pair<CSymbol*, pair<CSymbol*, CSymbol*>>>&  CCodeBlock_prime::GetPhis()
{
  return _phis;
}

void CCodeBlock_prime::AddPhi(CSymbol* dest, CSymbol* src1, CSymbol* src2)
{
  ADMIT;
}


ostream& CBlockTable::print(ostream &out, int indent) const
{
  string ind(indent, ' ');
  out << "<block table>";

  list<CBasicBlock*>::const_iterator it = _blocklist.begin();
  while (it != _blocklist.end()) {
    out << " " << (*it++);
  }

  out << " [initial : " << (GetInitBlock()->GetBlockNum()) << "] ";
  out << "[final :";
  it = _finblocks.begin();
  while (it != _finblocks.end()) {
    out << " " << ((*it++)->GetBlockNum());
  }
  out << "]";
  return out;
}
