#ifndef __SnuPL_IR2_H__
#define __SnuPL_IR2_H__

#include <iostream>
#include <list>
#include <vector>

#include "symtab.h"
#include "ir.h"

#define OpTailCall (OpNop + 1)
#define OpPhi (OpNop + 2)

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
    const string GetLabel_prime(void) const;

    /// @brief increase/decrease the reference counter
    int AddReference_prime(int ofs);

    /// @brief read the reference counter
    int GetRefCnt_prime(void) const;

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


    /// @name output
    /// @{

    /// @brief print the node to an output stream
    /// @param out output stream
    /// @param indent indentation
    virtual ostream&  print(ostream &out, int indent=0) const;

    /// @}

  protected:
    CBlockTable* _blktab; 
};


#endif // __SnuPL_IR2_H__
