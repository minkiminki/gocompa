#ifndef __SnuPL_IR2_H__
#define __SnuPL_IR2_H__

#include <iostream>
#include <list>
#include <vector>
#include <cassert>

#include "symtab.h"
#include "ir.h"
#include <boost/dynamic_bitset.hpp>
using namespace boost;

#define ADMIT (assert(false));

#define opTailCall ((EOperation)((int)opNop + 1))
#define opPhi ((EOperation)((int)opNop + 2))
#define opMov ((EOperation)((int)opNop + 3))

unsigned int GetSize_prime(const CType* ct);
int GetAlign_prime (const CType* ct);

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
/// @brief instruction class
///
/// base class for all instructions
///

class CBasicBlock : public CTac {
public:
  CBasicBlock(void);
  virtual ~CBasicBlock(void);

  list<CBasicBlock*>& GetPrevBlks(void);
  list<CBasicBlock*>& GetNextBlks(void);
  void AddPrevBlks(CBasicBlock *prev);
  void AddNextBlks(CBasicBlock *next);

  list<CBasicBlock*>& GetPreDoms(void);
  list<CBasicBlock*>& GetDoms(void);
  void AddPreDoms(CBasicBlock *prev);
  void AddDoms(CBasicBlock *next);
  void SetDoms(list<CBasicBlock*> doms);
  void SetPreDoms(list<CBasicBlock*> predoms);
  int DomsJoin(list<CBasicBlock*>& doms);
  int PreDomsJoin(list<CBasicBlock*>& predoms);
  list<CBasicBlock*>& GetDomFront(void);
  void AddDomFront(CBasicBlock *front);

  list<CTacInstr*>& GetInstrs(void);
  void AddInstr(CTacInstr* instr);
  void SetBlockNum(int blocknum);
  int GetBlockNum(void) const;
  list<CBasicBlock*>& ComputeDF(void);

  virtual ostream&  print(ostream &out, int indent=0) const;

protected:
  list<CBasicBlock*> _prevblks;
  list<CBasicBlock*> _nextblks;
  list<CTacInstr*> _instrs;
  int _blocknum;
  list<CBasicBlock*> _doms;
  list<CBasicBlock*> _predoms;
  list<CBasicBlock*> _domfrontier;
  bool dfcomputed;
};

class CBlockTable : public CTac {
public:
  CBlockTable(void);
  virtual ~CBlockTable(void);
  list<CBasicBlock*>& GetBlockList(void);
  void SetInitBlock(CBasicBlock* initblock);
  CBasicBlock* GetInitBlock(void) const;
  list<CBasicBlock*>& GetFinBlocks(void);
  list<CBasicBlock*>& GetFinPreDoms(void);
  void AddFinDom(CBasicBlock* block);
  void AddFinPreDom(CBasicBlock* block);
  void AddFinBlock(CBasicBlock* finblock);
  int AddBlock(CBasicBlock *block);
  void RemoveBlock(CBasicBlock *block);
  void BlockRenumber(void);
  void CombineBlock(CBasicBlock* blk, CBasicBlock* blk_next);
  virtual ostream&  print(ostream &out, int indent=0) const;


protected:
  list<CBasicBlock*> _blocklist;
  CBasicBlock* _initblock;
  list<CBasicBlock*> _finblocks;
  list<CBasicBlock*> _finpredoms;
  list<CBasicBlock*> _findoms;
  int maxblock;
};

class CTacInstr_prime : public CTacInstr {
  public:
    /// @name constructors/destructors
    /// @{

    /// @brief constructor
    /// @param op operation
    /// @param dst destination operand
    /// @param src1 first source operand
    /// @param src2 second source operand

  // CTacInstr_prime(EOperation op, CTac *dst, CTacAddr *src1, CTacAddr *src2);
    CTacInstr_prime(CTacInstr *instr);

    /// @brief destructor
    virtual ~CTacInstr_prime(void);

    CBasicBlock* GetFromBlock(void) const;
    void SetFromBlock(CBasicBlock* block);

  void SetRegister(CSymRegister *rg);
  CSymRegister *GetRegister();

    /// @}

    /// @name output
    /// @{

    /// @brief print the node to an output stream
    /// @param out output stream
    /// @param indent indentation
    virtual ostream&  print(ostream &out, int indent=0) const;

    /// @}

  protected:
    CBasicBlock *_block;
    dynamic_bitset<> liveness;
  CSymRegister *_rg;
};


//------------------------------------------------------------------------------
/// @brief label class
///
/// TAC class for labels
///

class CTacLabel_prime : public CTacInstr_prime {
  public:
    /// @name constructors/destructors
    /// @{

    /// @brief constructor
    /// @param label label name
  //    CTacLabel_prime(const string label);

  //    CTacLabel_prime(CTacInstr* instr);
    CTacLabel_prime(CTacInstr *instr, const string label, int refcnt);


    /// @brief destructor
    virtual ~CTacLabel_prime(void);

    /// @}


    /// @name properties
    /// @{

    /// @brief return the label
    const string GetLabel(void) const;

    /// @brief increase/decrease the reference counter
    int AddReference(int ofs);

    /// @brief read the reference counter
    int GetRefCnt(void) const;

    /// @}


    /// @name output
    /// @{

    /// @brief print the node to an output stream
    /// @param out output stream
    /// @param indent indentation
    virtual ostream& print(ostream &out, int indent=0) const;

    /// @}

  protected:
    const string _label;             ///< label
    int _refcnt;                     ///< reference counter
};


//------------------------------------------------------------------------------
/// @brief code block
///
/// base node class for code blocks
///

class CCodeBlock_prime : public CCodeBlock {
  public:
    /// @name constructors/destructors
    /// @{

    /// @brief constructor
    /// @param owner scope owning this block

    CCodeBlock_prime(CCodeBlock* cblock);

    /// @brief destructor
    virtual ~CCodeBlock_prime(void);

    /// @}

    CBlockTable* GetBlockTable() const;
    int GetStackSize() const;
    void SetStackSize(int size);
    int GetParamNum() const;
    void SetParamNum(int param_num);
    void AddInitialLabel(void);
    void SplitIf(CTacInstr_prime* instr);
    void SplitElse(CBasicBlock* bb_prev, CBasicBlock* bb);

  list<pair<CSymbol*, pair<CSymbol*, CSymbol*>>>& GetPhis();
  void AddPhi(CSymbol* dest, CSymbol* src1, CSymbol* src2);

    /// @name output
    /// @{

    /// @brief print the node to an output stream
    /// @param out output stream
    /// @param indent indentation
    virtual ostream&  print(ostream &out, int indent=0) const;

    /// @}

  protected:
    CBlockTable* _blktab;
    int _size;
    int _param_num;
  list<pair<CSymbol*, pair<CSymbol*, CSymbol*>>> _phis;
};


#endif // __SnuPL_IR2_H__
