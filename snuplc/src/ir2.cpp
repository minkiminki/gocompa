//------------------------------------------------------------------------------
/// @brief SnuPL intermediate representation
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2012/09/14 Bernhard Egger created
/// 2013/03/14 Bernhard Egger adapted to SnuPL/0
/// 2013/06/06 Bernhard Egger cleanup, added documentation
/// 2014/11/04 Bernhard Egger added opPos
/// 2016/04/01 Bernhard Egger adapted to SnuPL/1
///
/// @section license_section License
/// Copyright (c) 2012-2018, Computer Systems and Platforms Laboratory, SNU
/// All rights reserved.
///
/// Redistribution and use in source and binary forms,  with or without modifi-
/// cation, are permitted provided that the following conditions are met:
///
/// - Redistributions of source code must retain the above copyright notice,
///   this list of conditions and the following disclaimer.
/// - Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY  AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER  OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT,  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSE-
/// QUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF  SUBSTITUTE
/// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
/// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT
/// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY
/// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
/// DAMAGE.
//------------------------------------------------------------------------------

#include <iomanip>
#include <cassert>
#include <map>

#include "ir.h"
#include "ir2.h"
#include "ast.h"
using namespace std;


//------------------------------------------------------------------------------
// CTacInstr
//
CTacInstr_prime::CTacInstr_prime(CTacInstr *instr)
  : CTacInstr(instr->GetOperation(), instr->GetDest(), instr->GetSrc(1), instr->GetSrc(2)), _block(NULL), _prev(NULL), _next(NULL)
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
    CTacInstr *target = dynamic_cast<CTacInstr_prime*>(_dst);
    if (target != NULL) {
      if (relop) out << " goto ";

      CTacLabel_prime *l = dynamic_cast<CTacLabel_prime*>(target);
      if (l != NULL) out << l->GetLabel_prime();
      else out << target->GetId();
    }
  } else {
    out << "[CTacInstr: '" << _name << "']";
  }

  CBasicBlock *block = GetFromBlock();
  if(block != NULL) {
    out << " [" << (block->GetBlockNum()) << "]";
  }
  else{
    out << " [ no block info ]";
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
        //<< "  (refcnt: " << _refcnt << ")"
        ;
  }

  CBasicBlock *block = GetFromBlock();
  if(block != NULL) {
    out << " [" << (block->GetBlockNum()) << "]";
  }
  else{
    out << " [ no block info ]";
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

}

CCodeBlock_prime::~CCodeBlock_prime(void)
{
  //TODO same for normal CCodeBlock
}

ostream& CCodeBlock_prime::print(ostream &out, int indent) const
{
  string ind(indent, ' ');

  out << ind << "[[ " << GetName() << endl;

  list<CTacInstr*>::const_iterator it = _ops.begin();
  while (it != _ops.end()) {
    (*it++)->print(out, indent+2);
    out << endl;
  }

  out << ind << "]]" << endl;

  return out;
}

CBasicBlock::CBasicBlock(int blocknum)
  : _firstinstr(NULL), _lastinstr(NULL), _blocknum(blocknum)
{  
}

CBasicBlock::~CBasicBlock(void)
{
}

vector<CBasicBlock*>& CBasicBlock::GetPrevBlks(void)
{
  return _prevblks;
}

vector<CBasicBlock*>& CBasicBlock::GetNextBlks(void)
{
  return _nextblks;
}

void CBasicBlock::AddPrevBlks(CBasicBlock *prev)
{
  assert(prev != NULL);
  _prevblks.push_back(prev);
  return;
}

void CBasicBlock::AddNextBlks(CBasicBlock *next)
{
  assert(next != NULL);
  _nextblks.push_back(next);
  return;
}

CTacInstr *CBasicBlock::GetFirstInstr(void)
{
  return _firstinstr;
}

CTacInstr *CBasicBlock::GetLastInstr(void)
{
  return _lastinstr;
}

void CBasicBlock::SetFirstInstr(CTacInstr *first)
{
  _firstinstr = first;
}

void CBasicBlock::SetLastInstr(CTacInstr *last)
{
  _lastinstr = last;
}

void CBasicBlock::SetBlockNum(int blocknum)
{
  _blocknum = blocknum;
}

int CBasicBlock::GetBlockNum(void) const
{
  return _blocknum;
}

CBlockTable::CBlockTable(void)
  : maxblock(0)
{
}

CBlockTable::~CBlockTable(void)
{
}

vector<CBasicBlock*>& CBlockTable::GetBlockList(void)
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

CTacInstr* CTacInstr_prime::GetPrevInstr(void) const
{
  return _prev;
}

CTacInstr* CTacInstr_prime::GetNextInstr(void) const
{
  return _next;
}

void CTacInstr_prime::SetPrevInstr(CTacInstr *prev)
{
  _prev = prev;
}

void CTacInstr_prime::SetNextInstr(CTacInstr *next)
{
  _next = next;
}

CBasicBlock* CTacInstr_prime::GetFromBlock(void) const
{
  return _block;
}

void CTacInstr_prime::SetFromBlock(CBasicBlock* block)
{
  _block = block;
}

CBlockTable* CCodeBlock_prime::GetBlockTable()
{
  return _blktab;
}

