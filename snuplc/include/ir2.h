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

  list<CTacInstr*>& GetInstrs(void);
  void AddInstr(CTacInstr* instr);
  void SetBlockNum(int blocknum);
  int GetBlockNum(void) const;

  virtual ostream&  print(ostream &out, int indent=0) const;

protected:
  list<CBasicBlock*> _prevblks;
  list<CBasicBlock*> _nextblks;
  list<CTacInstr*> _instrs;
  int _blocknum;
};

class CBlockTable : public CTac {
public:
  CBlockTable(void);
  virtual ~CBlockTable(void);
  list<CBasicBlock*>& GetBlockList(void);
  void SetInitBlock(CBasicBlock* initblock);
  CBasicBlock* GetInitBlock(void) const;
  list<CBasicBlock*>& GetFinBlocks(void);
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
    void SplitIf(CTacInstr_prime* instr);
    void SplitElse(CBasicBlock* bb_prev, CBasicBlock* bb);

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


#endif // __SnuPL_IR2_H__
