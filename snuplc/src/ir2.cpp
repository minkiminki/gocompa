#include <iomanip>
#include <cassert>
#include <map>
#include <algorithm>
#include <list>
#include <set>

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
    if(findit == l2.end()){
      l1.erase(it_before);
      success = 1;
    }
  }
  return success;
}

//------------------------------------------------------------------------------
// CTacInstr
//

CTacInstr_prime::CTacInstr_prime(EOperation op, CTac *dst, CTacAddr *src1, CTacAddr *src2)
  : CTacInstr(op, dst, src1, src2), _block(NULL)
{
}

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


CTacPhi::CTacPhi(CSymbol* s)
  : CTacInstr_prime(opPhi, new CTacName(s), new CTacName(s), new CTacName(s))
{
}

void CTacPhi::SetSrcBlk(int num, CBasicBlock* blk)
{
  if (num == 1){
    src1_blk = blk;
  }
  else if (num == 2){
    src2_blk = blk;
  }
  else{
    assert(false);
  }
}

CBasicBlock* CTacPhi::GetSrcBlk(int num)
{
  if (num == 1){
    return src1_blk;
  }
  else if (num == 2){
    return src2_blk;
  }
  else{
    assert(false);
  }
}

ostream& CTacPhi::print(ostream &out, int indent) const
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
    if (_src1 != NULL) {
      out << _src1;
      {
	if(src1_blk != NULL) {
	  // out << "  [" << (block->GetBlockNum()) << "]";
	  out << "  [";
	  out.flags(ios::right);
	  out.width(3);
	  out << (src1_blk->GetBlockNum());
	  out << "]";
	}
	else{
	  out << "  [---]";
	}
      }
    }

    if (_src2 != NULL) {
      if (relop) out << " " << _op; else out << ",";
      out << " " << _src2;

      {
	if(src2_blk != NULL) {
	  // out << "  [" << (block->GetBlockNum()) << "]";
	  out << "  [";
	  out.flags(ios::right);
	  out.width(3);
	  out << (src2_blk->GetBlockNum());
	  out << "]";
	}
	else{
	  out << "  [---]";
	}
      }

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


// class CTacPhi : public CTacInstr_prime {
//   public:

//     CTacPhi(CTacInstr *instr);

//     void SetSrcBlk(int num, CBasicBlock* blk);
//     CBasicBlock* GetSrcBlk(int num);

//     /// @}

//     /// @name output
//     /// @{

//     /// @brief print the node to an output stream
//     /// @param out output stream
//     /// @param indent indentation
//     virtual ostream&  print(ostream &out, int indent=0) const;

//     /// @}

//   protected:
//     CBasicBlock *src1_blk;
//     CBasicBlock *src2_blk;
// };



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
  : tempinfo(0)
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

void CBasicBlock::SetPreDoms(list<CBasicBlock*> predoms)
{
  _predoms = predoms;
}

int CBasicBlock::DomsJoin(list<CBasicBlock*>& doms)
{
  return list_join(_doms, doms);
}

void CBasicBlock::ClearTempInfo()
{
  tempinfo = 0;
}

int CBasicBlock::PreDomsJoin(list<CBasicBlock*>& predoms)
{
  return list_join(_predoms, predoms);
}

list<CBasicBlock*>& CBasicBlock::ComputeDF(void)
{
  if(tempinfo > 0) return _domfrontier;

  list<CBasicBlock*>::const_iterator it = _nextblks.begin();
  while (it != _nextblks.end()){
    CBasicBlock* blk = *it++;
    if(blk==NULL) continue;

    list<CBasicBlock*>::iterator fit = find(_doms.begin(), _doms.end(), blk);
    if(fit == _doms.end()){
      nodup_insert(_domfrontier, blk);
    }
  }

  it = _doms.begin();
  while (it != _doms.end()){
    CBasicBlock* blk = *it++;
    assert(blk!=NULL);

    list<CBasicBlock*> df = blk->ComputeDF();

    list<CBasicBlock*>::iterator it2 = df.begin();
    while(it2 != df.end()){
      CBasicBlock* blk2 = *it2++;
      assert(blk2 != NULL);

      list<CBasicBlock*>::iterator fit = find(_doms.begin(), _doms.end(), blk2);
      if(fit == _doms.end()){
	nodup_insert(_domfrontier, blk2);
      }
    }
  }

  tempinfo = 1;
  return _domfrontier;
}

list<CTacInstr*>& CBasicBlock::GetPhis(void)
{
  return _phis;
}

list<pair<const CSymbol*, const CSymbol*>>& CBasicBlock::GetBackPhis()
{
  return _backphis;
}

void CBasicBlock::AddBackPhi(const CSymbol* dst, const CSymbol* src)
{
  _backphis.push_front(pair<const CSymbol*, const CSymbol*>(dst, src));
}

void CBasicBlock::AddPhi(list<CBasicBlock*>& worklist, CSymbol* s)
{
  if(tempinfo >= 2) return;
  // CTacName* ndst = new CTacName(s);
  // CTacName* nsrc1 = new CTacName(s);
  // CTacName* nsrc2 = new CTacName(s);

  CTacPhi *instr_new = new CTacPhi(s);

  // CTacInstr_prime *instr_new = new CTacInstr_prime(opPhi, ndst, nsrc1, nsrc2);

  // CTacInstr *_instr_new = new CTacInstr(opNop, ndst, nsrc1, nsrc2);
  // CTacInstr_prime *instr_new = new CTacInstr_prime(_instr_new);
  // instr_new->SetOperation(opPhi);

  _phis.push_front(instr_new);
  instr_new->SetFromBlock(this);

  if(tempinfo == 0){
    nodup_insert(worklist, this);
  }
  tempinfo = 2;
}

void CBasicBlock::ComputePhi(list<CBasicBlock*>& worklist, CSymbol* s)
{
  list<CBasicBlock*>::const_iterator bit = _domfrontier.begin();
  while (bit != _domfrontier.end()) {
    CBasicBlock* blk = *bit++;
    blk->AddPhi(worklist, s);
  }
}

void CBasicBlock::SetTempInfo(int temp)
{
  tempinfo = temp;
}

int CBasicBlock::GetTempInfo()
{
  return tempinfo;
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

CTacInstr* CBasicBlock::CheckAssign(CSymbol* s) const
{

  list<CTacInstr*>::const_reverse_iterator it = _instrs.rbegin();
  while (it != _instrs.rend()){
    CTacInstr* instr = *it++;
    assert(instr != NULL);
    if(instr->GetOperation() == opAssign){
      CTacName* n = dynamic_cast<CTacName*>(instr->GetDest());
      assert(n != NULL);
      if(n->GetSymbol() == s){
	if(dynamic_cast<CTacReference*>(n) == NULL){
	  return instr;
	}
      }
    }
  }
  return NULL;
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
  out << " ||";
  it = _domfrontier.begin();
  while (it != _domfrontier.end()){
    CBasicBlock* blk = *it++;
    if(blk == NULL){
      out << " OUT";
    }
    else{
      out << " " << (blk->GetBlockNum());
    }
  }

  // for debugging
  list<CTacInstr*>::const_iterator pit = _phis.begin();
  while (pit != _phis.end()) {
    (*pit++)->print(out, indent+2);
    out << endl;
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

void CBlockTable::AddFinDom(CBasicBlock* block)
{
  nodup_insert(_findoms, block);
}

CBasicBlock* CBlockTable::GetInitBlock(void) const
{
  return _initblock;
}

list<CBasicBlock*>& CBlockTable::GetFinBlocks(void)
{
  return _finblocks;
}

void CBlockTable::BlockRenumber(const list<CTacInstr*>& instrs)
{

  maxblock = 0;
  _blocklist.clear();

  set<CBasicBlock*> inserted;
  list<CTacInstr*>::const_iterator it = instrs.begin();
  while(it != instrs.end()){
    CTacInstr_prime* instr = dynamic_cast<CTacInstr_prime*>(*it++);
    assert(instr != NULL);
    CBasicBlock* cb = instr->GetFromBlock();
    if(cb == NULL){
      continue;
      // cout << instr;./
      // _P1;
      // ADMIT;
    }

    if(inserted.find(cb) != inserted.end())
      continue;
    else {
      inserted.insert(cb);
      AddBlock(cb);
    }
  }

  // maxblock = 0;
  // list<CBasicBlock*>::const_iterator it = _blocklist.begin();
  // while(it != _blocklist.end()){
  //   CBasicBlock* cb = *it++;
  //   assert(cb != NULL);
  //   cb->SetBlockNum(++maxblock);
  // }


}

void CBlockTable::ClearTempInfos(void)
{
  list<CBasicBlock*>::const_iterator it = _blocklist.begin();
  while(it != _blocklist.end()){
    CBasicBlock* cb = *it++;
    assert(cb != NULL);
    cb->ClearTempInfo();
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

void CCodeBlock_prime::RemoveNop()
{
  list<CTacInstr*>::iterator it = _ops.begin();
  while (it != _ops.end()) {
    list<CTacInstr*>::iterator iit = it++;
    CTacInstr* instr = *iit;
    if(instr->GetOperation() == opNop){
      _ops.erase(iit);
    }
  }
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

void CCodeBlock_prime::AddInitialLabel(void)
{
  CTacLabel *_lb = _owner->CreateLabel("initial");
  CTacLabel_prime *lb = new CTacLabel_prime(_lb, _lb->GetLabel(),_lb->GetRefCnt());
  _ops.push_front(lb);
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

  CTacInstr_prime *instr_new = new CTacInstr_prime(opGoto, instr, NULL, NULL);
  // CTacInstr *_instr_new = new CTacInstr(opNop, instr, NULL, NULL);
  // CTacInstr_prime *instr_new = new CTacInstr_prime(_instr_new);
  // instr_new->SetOperation(opGoto);

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

void CCodeBlock_prime::SSA_out()
{
  list<CBasicBlock*>::const_iterator bit = _blktab->GetBlockList().begin();
  while (bit != _blktab->GetBlockList().end()) {
    CBasicBlock* blk = *bit++;
    assert(blk!=NULL);

    list<CTacInstr*>::const_iterator pit = blk->GetPhis().begin();
    while (pit != blk->GetPhis().end()) {
      CTacPhi* phi = dynamic_cast<CTacPhi*>(*pit++);
      assert(phi!=NULL);

      {
	CBasicBlock* blk1 = phi->GetSrcBlk(1);
	assert(blk1!=NULL);

	assert(blk1->GetInstrs().rbegin() != blk1->GetInstrs().rend());
	CTacInstr *instr = *(blk1->GetInstrs().rbegin());
	assert(instr != NULL);
	list<CTacInstr*>::iterator fit = find(_ops.begin(), _ops.end(), instr);
	assert(fit != _ops.end());

	list<CTacInstr*>::iterator instrend = blk1->GetInstrs().end();

	if(instr->GetOperation() != opGoto){
	  fit = next(fit);
	  instrend = --instrend;
	}

	CTacAddr* src1;
	{
	  CTacName* _s_src = dynamic_cast<CTacName*>(phi->GetSrc(1));
	  if(_s_src != NULL){
	    src1 = new CTacTemp(_s_src->GetSymbol());
	  }
	  else{
	    CTacConst* _s_src = dynamic_cast<CTacConst*>(phi->GetSrc(1));
	    assert(_s_src != NULL);
	    src1 = new CTacConst(_s_src->GetValue());
	  }
	}

	const CSymbol * s_dest = (dynamic_cast<CTacName*>(phi->GetDest()))->GetSymbol();

	CTacInstr_prime *instr_new = new CTacInstr_prime(opAssign, new CTacTemp(s_dest),
							 src1, NULL);

	instr_new->SetFromBlock(blk1);
	(blk1->GetInstrs()).insert(instrend, instr_new);
	_ops.insert(fit, instr_new);
      }

      {
	CBasicBlock* blk2 = phi->GetSrcBlk(2);
	assert(blk2!=NULL);

	assert(blk2->GetInstrs().rbegin() != blk2->GetInstrs().rend());
	CTacInstr *instr = *(blk2->GetInstrs().rbegin());
	assert(instr != NULL);
	list<CTacInstr*>::iterator fit = find(_ops.begin(), _ops.end(), instr);
	assert(fit != _ops.end());

	list<CTacInstr*>::iterator instrend = blk2->GetInstrs().end();

	if(instr->GetOperation() != opGoto){
	  fit = next(fit);
	  instrend = --instrend;
	}

	CTacAddr* src2;
	{
	  CTacName* _s_src = dynamic_cast<CTacName*>(phi->GetSrc(2));
	  if(_s_src != NULL){
	    src2 = new CTacTemp(_s_src->GetSymbol());
	  }
	  else{
	    CTacConst* _s_src = dynamic_cast<CTacConst*>(phi->GetSrc(2));
	    assert(_s_src != NULL);
	    src2 = new CTacConst(_s_src->GetValue());
	  }
	}

	const CSymbol * s_dest = (dynamic_cast<CTacName*>(phi->GetDest()))->GetSymbol();

	CTacInstr_prime *instr_new = new CTacInstr_prime(opAssign, new CTacTemp(s_dest),
							 src2, NULL);

	instr_new->SetFromBlock(blk2);
	(blk2->GetInstrs()).insert(instrend, instr_new);
	_ops.insert(fit, instr_new);
      }
    }
  }
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
