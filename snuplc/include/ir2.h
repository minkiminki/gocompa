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

  CBasicBlock(int blocknum);
  virtual ~CBasicBlock(void);

  vector<CBasicBlock*>& GetPrevBlks(void);
  vector<CBasicBlock*>& GetNextBlks(void);
  void AddPrevBlks(CBasicBlock *prev);
  void AddNextBlks(CBasicBlock *next);
  CTacInstr* GetFirstInstr(void);
  CTacInstr* GetLastInstr(void);
  void SetFirstInstr(CTacInstr *first);
  void SetLastInstr(CTacInstr *last);
  void SetBlockNum(int blocknum);
  int GetBlockNum(void) const;

protected:
  vector<CBasicBlock*> _prevblks;
  vector<CBasicBlock*> _nextblks;
  CTacInstr *_firstinstr;
  CTacInstr *_lastinstr;
  int _blocknum;
};

class CBlockTable {
public:
  CBlockTable(void);
  virtual ~CBlockTable(void);
  vector<CBasicBlock*>& GetBlockList(void);
  int AddBlock(CBasicBlock *block);
protected:
  vector<CBasicBlock*> _blocklist;
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

    CTacInstr* GetPrevInstr(void) const;
    CTacInstr* GetNextInstr(void) const;
    void SetPrevInstr(CTacInstr *prev);
    void SetNextInstr(CTacInstr *next);
    CBasicBlock* GetFromBlock(void) const;
    void SetFromBlock(CBasicBlock*);
  
    /// @}

    /// @name output
    /// @{

    /// @brief print the node to an output stream
    /// @param out output stream
    /// @param indent indentation
    virtual ostream&  print(ostream &out, int indent=0) const;

    /// @}

  protected:
    CTacInstr *_prev;
    CTacInstr *_next;
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

    CBlockTable* GetBlockTable();


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
