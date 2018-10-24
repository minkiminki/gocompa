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

    /// @}

    /// @name output
    /// @{

    /// @brief print the node to an output stream
    /// @param out output stream
    /// @param indent indentation
    virtual ostream&  print(ostream &out, int indent=0) const;

    /// @}

  protected:
    int TODO;
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


    /// @name output
    /// @{

    /// @brief print the node to an output stream
    /// @param out output stream
    /// @param indent indentation
    virtual ostream&  print(ostream &out, int indent=0) const;

    /// @}

  protected:
    int TODO;
};


#endif // __SnuPL_IR2_H__
