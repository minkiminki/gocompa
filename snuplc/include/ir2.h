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
#define _P1 (printf("------------------------pass first  point---------------------\n1111111111111111111111111111111111111111111111111111111111111\n"));
#define _P2 (printf("------------------------pass second point---------------------\n2222222222222222222222222222222222222222222222222222222222222\n"));

#define opTailCall ((EOperation)((int)opNop + 1))
#define opPhi ((EOperation)((int)opNop + 2))
#define opMov ((EOperation)((int)opNop + 3))
#define opDIM ((EOperation)((int)opNop + 4))
#define opDOFS ((EOperation)((int)opNop + 5))

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

template<typename T>
int list_pop(list<T>& l, T* ret){
  if(l.empty()){
    return -1;
  }
  else{
    *ret = l.front();
    l.pop_front();
    return 1;
  }
}

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

template<typename T>
int list_add(list<T>& l1, list<T>& l2){
  int success = 0;
  typename list<T>::iterator it = l2.begin();
  while(it != l2.end()){
    T key = *it++;
    if(!nodup_insert(l1, key)) success = 1;
  }
  return success;
}

template<typename T>
int list_substract(list<T>& l1, list<T>& l2){
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
  void ClearTempInfo(void);
  list<CTacInstr*>& GetPhis();
  list<pair<const CSymbol*, const CSymbol*>>& GetBackPhis();
  void AddBackPhi(const CSymbol* dst, const CSymbol* src);
  void AddPhi(list<CBasicBlock*>& worklist, CSymbol* s);
  void ComputePhi(list<CBasicBlock*>& worklist, CSymbol* s);
  void SetTempInfo(int temp);
  int GetTempInfo(void);

  list<CTacInstr*>& GetInstrs(void);
  void AddInstr(CTacInstr* instr);
  void SetBlockNum(int blocknum);
  int GetBlockNum(void) const;
  list<CBasicBlock*>& ComputeDF(void);
  CTacInstr* CheckAssign(CSymbol* s) const;

  virtual ostream&  print(ostream &out, int indent=0) const;

protected:
  list<CBasicBlock*> _prevblks;
  list<CBasicBlock*> _nextblks;
  list<CTacInstr*> _instrs;
  int _blocknum;
  list<CBasicBlock*> _doms;
  list<CBasicBlock*> _predoms;
  list<CBasicBlock*> _domfrontier;
  int tempinfo;
  list<CTacInstr*> _phis;
  list<pair<const CSymbol*, const CSymbol*>> _backphis;
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
  void RemoveBlock(CCodeBlock* owner, CBasicBlock *block);
  void BlockRenumber(const list<CTacInstr*>& instrs);
  void CombineBlock(CBasicBlock* blk, CBasicBlock* blk_next);
  void ClearTempInfos(void);
  Liveness* GetLiveness(void);
  virtual ostream&  print(ostream &out, int indent=0) const;


protected:
  list<CBasicBlock*> _blocklist;
  CBasicBlock* _initblock;
  list<CBasicBlock*> _finblocks;
  list<CBasicBlock*> _finpredoms;
  list<CBasicBlock*> _findoms;
  int maxblock;
  Liveness* _liveness;
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
    CTacInstr_prime(EOperation op, CTac *dst, CTacAddr *src1, CTacAddr *src2);

    /// @brief destructor
    virtual ~CTacInstr_prime(void);

    CBasicBlock* GetFromBlock(void) const;
    void SetFromBlock(CBasicBlock* block);

  list<const CSymbol*>& GetLiveVars(void);
  void SetLiveVars(list<const CSymbol*>& live_vars);

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
    list<const CSymbol*> _live_vars;
};

class CTacPhi : public CTacInstr_prime {
  public:

    CTacPhi(CSymbol* s);

    void SetSrcBlk(int num, CBasicBlock* blk);
    CBasicBlock* GetSrcBlk(int num);

    /// @}

    /// @name output
    /// @{

    /// @brief print the node to an output stream
    /// @param out output stream
    /// @param indent indentation
    virtual ostream&  print(ostream &out, int indent=0) const;

    /// @}

  protected:
    CBasicBlock *src1_blk;
    CBasicBlock *src2_blk;
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
    void SSA_out();
    void RemoveNop();

  // TODO

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

};

class Liveness {
 public:
  Liveness(void);
  // ~Liveness(void);

  // const CSymbol** GetParamRegs(void);
  // const CSymbol* GetCallerSave(int index);
  map<CBasicBlock*, list<const CSymbol*>> & GetUses(int index);
  map<const CSymbol*, list<const CSymbol*>> & GetAssignGraph(void);
  map<const CSymbol*, list<const CSymbol*>> & GetLiveGraph(void);
  map<const CSymbol*, ERegister> & GetAllocated(void);
  list<const CSymbol*> & GetSymbList(void);

  const CSymbol* GetDeadCalleeSave(int index);
  const CSymbol* GetDeadParam(int index);
  // const CSymbol* CreateDeadCalleeSave(int index);
  // const CSymbol* CreateDeadParam(int index);
  const CSymbol* CreateArgReg(int index);
  const CSymbol* CreateParamReg(int index);
  // list<CSymbol*> GetDeadRegs(void);
  list<const CSymbol*>& GetTempRegs(void);
  void debug_print(void);
  int Allocate(void);

 protected:
  // list<CSymbol*> & _deadregs;
  list<const CSymbol*> _tempregs;
  map<CBasicBlock*, list<const CSymbol*>> _uses1;
  map<CBasicBlock*, list<const CSymbol*>> _uses2;
  const CSymbol* _param_regs[6];
  const CSymbol* _caller_save1;
  const CSymbol* _caller_save2;
  int deadnum;
  int tempnum;
  map<const CSymbol*, list<const CSymbol*>> _assign_graph;
  map<const CSymbol*, list<const CSymbol*>> _live_graph;
  map<const CSymbol*, ERegister> _allocated;
  list<const CSymbol*> _symblist;

};

#endif // __SnuPL_IR2_H__
