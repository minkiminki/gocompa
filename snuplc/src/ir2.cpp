#include <iomanip>
#include <cassert>
#include <map>
#include <algorithm>
#include <list>

#include "ir.h"
#include "ir2.h"
#include "ast.h"
using namespace std;


template<typename T>
int erase_success(list<T>& l, T key){
  typename list<T>::iterator findit = find(l.begin(), l.end(), key);
  if(findit != l.end()){
    l.erase(findit);
    return 0;
  }
  else{
    return -1;
  }
}

template<typename T>
int nodup_insert(list<T>& l, T key){
  typename list<T>::iterator findit = find(l.begin(), l.end(), key);
  if(findit != l.end()){
    return 1;
  }
  else{
    l.push_back(key);
    return 0;
  }
}

//------------------------------------------------------------------------------
// CTacInstr
//
CTacInstr_prime::CTacInstr_prime(CTacInstr *instr)
  : CTacInstr(instr->GetOperation(), instr->GetDest(), instr->GetSrc(1), instr->GetSrc(2)), _block(NULL)
{
}

CTacInstr_prime::~CTacInstr_prime(void)
{
  if (IsBranch()) {
    CTacLabel_prime *lbl = dynamic_cast<CTacLabel_prime*>(_dst);
    assert(lbl != NULL);
    lbl->AddReference_prime(-1);
  }
}

ostream& CTacInstr_prime::print(ostream &out, int indent) const
{

  CTacInstr::print(out, indent);
  // string ind(indent, ' ');

  // out << ind << right << dec << setw(3) << _id << ": ";

  // if (_name == "") {
  //   bool relop = IsRelOp(GetOperation());

  //   out << "    " << left << setw(6);
  //   if (relop) out << "if"; else out << _op;
  //   out << " ";
  //   CTacAddr *adr = dynamic_cast<CTacAddr*>(_dst);
  //   if (adr != NULL) out << _dst << " <- ";
  //   if (_src1 != NULL) out << _src1;
  //   if (_src2 != NULL) {
  //     if (relop) out << " " << _op; else out << ",";
  //     out << " " << _src2;
  //   }
  //   CTacInstr *target = dynamic_cast<CTacInstr_prime*>(_dst);
  //   if (target != NULL) {
  //     if (relop) out << " goto ";

  //     CTacLabel_prime *l = dynamic_cast<CTacLabel_prime*>(target);
  //     if (l != NULL) out << l->GetLabel_prime();
  //     else out << target->GetId();
  //   }
  // } else {
  //   out << "[CTacInstr: '" << _name << "']";
  // }

  CBasicBlock *block = GetFromBlock();
  if(block != NULL) {
    out << "  [" << (block->GetBlockNum()) << "]";
  }
  else{
    out << "  [no block info]";
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

/*
CTacLabel_prime::CTacLabel_prime(CTacInstr *instr)
  : CTacInstr_prime(instr)
{
  CTacLabel *lbl = dynamic_cast<CTacLabel*>(instr);
  assert(lbl != NULL);
  _label = "";
  //  const string a = "";
  //  _label = lbl->GetLabel();
  _refcnt = lbl->GetRefCnt();
}
*/

CTacLabel_prime::~CTacLabel_prime(void)
{
}

const string CTacLabel_prime::GetLabel_prime(void) const
{
  return _label;
}

int CTacLabel_prime::AddReference_prime(int ofs)
{
  _refcnt += ofs;
  return _refcnt;
}

int CTacLabel_prime::GetRefCnt_prime(void) const
{
  return _refcnt;
}

ostream& CTacLabel_prime::print(ostream &out, int indent) const
{
  if (true || GetRefCnt_prime() > 0) {
    string ind(indent, ' ');

    out << ind << right << dec << setw(3) << _id << ": "
        << left << _label << ":"
        // << "  (refcnt: " << _refcnt << ")"
        ;
  }

  CBasicBlock *block = GetFromBlock();
  if(block != NULL) {
    out << " [" << (block->GetBlockNum()) << "]";
  }
  else{
    out << " [no block info]";
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
  // list<CBasicBlock*>::iterator it = find(_blocklist.begin(), _blocklist.end(), blk);
  // assert(it != _blocklist.end());
  // _blocklist.erase(it);

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
  // assert(finblock != NULL);
  // _finblocks.push_back(finblock);

  // assert(finblock != NULL);
  // list<CBasicBlock*>::iterator it = find(_finblocks.begin(), _finblocks.end(), finblock);
  // if (it == _finblocks.end()) {
  //   _finblocks.push_back(finblock);
  // }
  return;
}

CBasicBlock* CBlockTable::GetInitBlock(void) const
{
  return _initblock;
}

list<CBasicBlock*>& CBlockTable::GetFinBlocks(void)
{
  return _finblocks;
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

