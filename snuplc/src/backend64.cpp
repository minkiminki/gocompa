//------------------------------------------------------------------------------
/// @brief SnuPL backend
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2012/11/28 Bernhard Egger created
/// 2013/06/09 Bernhard Egger adapted to SnuPL/0
/// 2016/04/04 Bernhard Egger adapted to SnuPL/1
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

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "backend.h"
using namespace std;

string param_regs[6] = {"%rdi\0","%rsi\0","%rdx\0","%rcx\0","%r8\0","%r9\0"};
string callee_regs[5] = {"%rbx\0","%r12\0","%r13\0","%r14\0","%r15\0"};
string caller_regs[2] = {"%r10\0", "%r11\0"};
static bool isTailCall = false;
string all_regs[4][14] = {
  {"%al", "%bl", "%cl", "%dl", "%sil", "%dil", "%r8b", "%r9b", "%r10b",
    "%r11b", "%r12b", "%r13b", "%r14b", "%r15b"},
  {"%ax", "%bx", "%cx", "%dx", "%si", "%di", "%r8w", "%r91", "%r10w",
    "%r11w", "%r12w", "%r13w", "%r14w", "%r15w"},
  {"%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi", "%r8d", "%r9d", "%r10d",
    "%r11d", "%r12d", "%r13d", "%r14d", "%r15d"},
  {"%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "%r8", "%r9", "%r10",
    "%r11", "%r12", "%r13", "%r14", "%r15"}
};

static int tmp_label_count = 0;

unsigned int GetSize_prime (const CType* ct)
{
  if(ct->IsPointer())
    return 8;
  else
    return ct->GetSize();
}

int GetAlign_prime (const CType* ct)
{
  if(ct->IsPointer())
    return 8;
  else
    return ct->GetAlign();
}

// find register type - 만든 이유는 Load 인자를 인티저로 바꾸기 귀찮았기 때문
int getRegType(string reg)
{
  if(reg.compare("%rax") == 0) return 0;
  if(reg.compare("%rbx") == 0) return 1;
  if(reg.compare("%rcx") == 0) return 2;
  if(reg.compare("%rdx") == 0) return 3;
  if(reg.compare("%rsi") == 0) return 4;
  if(reg.compare("%rdi") == 0) return 5;
  if(reg.compare("%r8") == 0) return 6;
  if(reg.compare("%r9") == 0) return 7;
  if(reg.compare("%r10") == 0) return 8;
  if(reg.compare("%r11") == 0) return 9;
  if(reg.compare("%r12") == 0) return 10;
  if(reg.compare("%r13") == 0) return 11;
  if(reg.compare("%r14") == 0) return 12;
  if(reg.compare("%r15") == 0) return 13;
  if(reg.compare("%r16") == 0) return 14;
  return stoi(reg);
}

// get register string based on operand's size
// wow :(
string getRegister(string reg, int size)
{
  int regNum = getRegType(reg);
  switch(size){
    case 1: return all_regs[0][regNum];
    case 2: return all_regs[1][regNum];
    case 4: return all_regs[2][regNum];
    case 8: return all_regs[3][regNum];
    default: return "";
  }
}

// CBackendx86_64
//
CBackendx86_64::CBackendx86_64(ostream &out)
  : CBackend(out), _curr_scope(NULL)
{
  _ind = string(4, ' ');
}

CBackendx86_64::~CBackendx86_64(void)
{
}

void CBackendx86_64::EmitHeader(void)
{
  _out << "##################################################" << endl
    << "# " << _m->GetName() << endl
    << "#" << endl
    << endl;
}

void CBackendx86_64::EmitCode(void)
{
  _out << _ind << "#-----------------------------------------" << endl
    << _ind << "# text section" << endl
    << _ind << "#" << endl
    << _ind << ".text" << endl
    << _ind << ".align 4" << endl
    << endl
    << _ind << "# entry point and pre-defined functions" << endl
    << _ind << ".global main" << endl
    << _ind << ".extern DIM" << endl
    << _ind << ".extern DOFS" << endl
    << _ind << ".extern ReadInt" << endl
    << _ind << ".extern WriteInt" << endl
    << _ind << ".extern WriteStr" << endl
    << _ind << ".extern WriteChar" << endl
    << _ind << ".extern WriteLn" << endl
    << endl;

  // emit subscopes (=procedures/functions) first
  vector<CScope*>::const_iterator sit = _m->GetSubscopes().begin();
  while (sit != _m->GetSubscopes().end()) EmitScope(*sit++);

  EmitScope(_m);

  _out << _ind << "# end of text section" << endl
    << _ind << "#-----------------------------------------" << endl
    << endl;
}

void CBackendx86_64::EmitData(void)
{
  _out << _ind << "#-----------------------------------------" << endl
    << _ind << "# global data section" << endl
    << _ind << "#" << endl
    << _ind << ".data" << endl
    << _ind << ".align 4" << endl
    << endl;

  EmitGlobalData(_m);

  _out << _ind << "# end of global data section" << endl
    << _ind << "#-----------------------------------------" << endl
    << endl;
}

void CBackendx86_64::EmitFooter(void)
{
  _out << _ind << ".end" << endl
    << "##################################################" << endl;
}

void CBackendx86_64::SetScope(CScope *scope)
{
  _curr_scope = scope;
}

CScope* CBackendx86_64::GetScope(void) const
{
  return _curr_scope;
}

void CBackendx86_64::EmitScope(CScope *scope)
{
  assert(scope != NULL);

  string label;

  if (scope->GetParent() == NULL) label = "main";
  else label = scope->GetName();

  // label
  _out << _ind << "# scope " << scope->GetName() << endl
    << label << ":" << endl;

  // compute the size of locals
  CSymtab *st = scope->GetSymbolTable();
  assert(st != NULL);

  CCodeBlock_prime *cb = dynamic_cast<CCodeBlock_prime*>(scope->GetCodeBlock());
  assert(cb != NULL);
  // cb->SetStackSize(ComputeStackOffsets(st, 8, -12));

  size_t size = cb->GetStackSize();
  int param_num = cb->GetParamNum();

  StackDump(st);

  // size_t size = ComputeStackOffsets(st, 8, -12);

  // prologue
  _out << _ind << "# prologue" << endl;
  EmitInstruction("pushq", "%rbp");
  EmitInstruction("movq", "%rsp, %rbp");
  // currently push/pop all regs
  const boost::dynamic_bitset<> callee_used_regs(5, 31ul);
  EmitCalleePush(callee_used_regs);
  EmitParamPush(param_num);
  //EmitInstruction("pushl", "%rbx", "save callee saved registers");
  //EmitInstruction("pushl", "%rsi");
  //EmitInstruction("pushl", "%rdi");
  EmitInstruction("subq", Imm(size) + ", %rsp", "make room for locals");

  // clear stack
  //modified
  size_t lsize = size/8;
  if (lsize > 4) {
    _out << endl;
    EmitInstruction("cld", "", "memset local stack area to 0");
    EmitInstruction("xorq", "%rax, %rax");
    EmitInstruction("mov", Imm(lsize) + ", %rcx");
    EmitInstruction("mov", "%rsp, %rdi");
    EmitInstruction("rep", "stosq");
  } else if (lsize > 0) {
    _out << endl;
    EmitInstruction("xorq", "%rax, %rax", "memset local stack area to 0");
    do {
      lsize--;
      ostringstream o;
      o << "%rax, " << dec << lsize*8 << "(%rsp)";
      EmitInstruction("movq", o.str());
    } while (lsize > 0);
  }

  // initialize local arrays
  EmitLocalData(scope);

  // emit code
  _out << endl
    << _ind << "# function body" << endl;

  SetScope(scope);
  // CCodeBlock *cb = scope->GetCodeBlock();

  if (cb != NULL) EmitCodeBlock(cb);

  if(isTailCall==false)
    EmitEpilogue();

}

void CBackendx86_64::EmitEpilogue()
{
  // epilogue
  CScope *cs = GetScope();
  assert(cs != NULL);
  CCodeBlock_prime *cb = dynamic_cast<CCodeBlock_prime*>(cs->GetCodeBlock());
  size_t size = cb->GetStackSize();
  int param_num = cb->GetParamNum();
  const boost::dynamic_bitset<> callee_used_regs(5, 31ul);

  _out << endl;
  if(isTailCall==false) {
    _out << Label("exit") << ":" << endl;
    _out << _ind << "# epilogue" << endl;
  }
  EmitInstruction("addq", Imm(size) + ", %rsp", "remove locals");
  //EmitInstruction("popq", "%rdi");
  //EmitInstruction("popq", "%rsi");
  //EmitInstruction("popq", "%rbx");
  // currently push/pop all regs
  int param_size = (param_num <= 6) ? param_num*8 : 6*8;
  EmitCalleePop(callee_used_regs);
  EmitInstruction("addq", Imm(param_size) + ", %rsp", "remove params");
  EmitInstruction("popq", "%rbp");
  if(isTailCall==false)
    EmitInstruction("ret");
  _out << endl;
  isTailCall = false;
}

void CBackendx86_64::EmitGlobalData(CScope *scope)
{
  assert(scope != NULL);

  // emit the globals for the current scope
  CSymtab *st = scope->GetSymbolTable();
  assert(st != NULL);

  bool header = false;

  vector<CSymbol*> slist = st->GetSymbols();

  _out << dec;

  size_t size = 0;

  for (size_t i=0; i<slist.size(); i++) {
    CSymbol *s = slist[i];
    const CType *t = s->GetDataType();

    if (s->GetSymbolType() == stGlobal) {
      if (!header) {
        _out << _ind << "# scope: " << scope->GetName() << endl;
        header = true;
      }

      // insert alignment only when necessary
      if ((GetAlign_prime(t) > 1) && (size % GetAlign_prime(t) != 0)) {
        size += GetAlign_prime(t) - size % GetAlign_prime(t);
        _out << setw(4) << " " << ".align "
          << right << setw(3) << GetAlign_prime(t) << endl;
      }

      _out << left << setw(36) << s->GetName() + ":" << "# " << t << endl;

      if (t->IsArray()) {
        const CArrayType *a = dynamic_cast<const CArrayType*>(t);
        assert(a != NULL);
        int dim = a->GetNDim();

        _out << setw(4) << " "
          << ".long " << right << setw(4) << dim << endl;

        for (int d=0; d<dim; d++) {
          assert(a != NULL);

          _out << setw(4) << " "
            << ".long " << right << setw(4) << a->GetNElem() << endl;

          a = dynamic_cast<const CArrayType*>(a->GetInnerType());
        }
      }

      const CDataInitializer *di = s->GetData();
      if (di != NULL) {
        const CDataInitString *sdi = dynamic_cast<const CDataInitString*>(di);
        assert(sdi != NULL);  // only support string data initializers for now

        _out << left << setw(4) << " "
          << ".asciz " << '"' << sdi->GetData() << '"' << endl;
      } else {
        _out  << left << setw(4) << " "
          << ".skip " << dec << right << setw(4) << t->GetDataSize()
          << endl;
      }

      size += GetSize_prime(t);
    }
  }

  _out << endl;

  // emit globals in subscopes (necessary if we support static local variables)
  vector<CScope*>::const_iterator sit = scope->GetSubscopes().begin();
  while (sit != scope->GetSubscopes().end()) EmitGlobalData(*sit++);
}

void CBackendx86_64::EmitLocalData(CScope *scope)
{
  assert(scope != NULL);

  // emit the globals for the current scope
  CSymtab *st = scope->GetSymbolTable();
  assert(st != NULL);

  vector<CSymbol*> slist = st->GetSymbols();

  _out << dec;

  for (size_t i=0; i<slist.size(); i++) {
    CSymbol *s = slist[i];
    const CType *t = s->GetDataType();

    if ((s->GetSymbolType() == stLocal) && (t->IsArray())) {
      const CArrayType *a = dynamic_cast<const CArrayType*>(t);
      assert(a != NULL);

      int ofs = 0;
      int dim = a->GetNDim();

      ostringstream dst;
      assert(!s->isInReg());
      //dst << s->GetOffset()+ofs << "(" << s->GetBaseRegister() << ")";
      dst << s->GetOffset()+ofs << "(" << "%rbp" << ")";
      // since base reg is not assigned, put all local data into reg now

      ofs += 4;

      ostringstream comment;
      comment << "local array '" << s->GetName() << "': "
        << dim << " dimensions";

      EmitInstruction("movq", Imm(dim) + "," + dst.str(), comment.str());

      for (int d=0; d<dim; d++) {
        assert(a != NULL);

        ostringstream dst;
        assert(!s->isInReg());
        dst << s->GetOffset()+ofs << "(" << "%rbp" << ")";
        //        dst << s->GetOffset()+ofs << "(" << s->GetBaseRegister() << ")";
        ofs += 4;

        ostringstream comment;
        comment << "  dimension " << d+1 << ": "
          << a->GetNElem() << " elements";

        EmitInstruction("movq", Imm(a->GetNElem()) + "," + dst.str(),
            comment.str());

        a = dynamic_cast<const CArrayType*>(a->GetInnerType());
      }
    }
  }
}

void CBackendx86_64::EmitCodeBlock(CCodeBlock *cb)
{
  assert(cb != NULL);

  const list<CTacInstr*> &instr = cb->GetInstr();
  list<CTacInstr*>::const_iterator it = instr.begin();

  while (it != instr.end()) EmitInstruction(*it++);
}

//working
void CBackendx86_64::EmitOperation(CTacInstr *i, string comment)
{
  int reg_count = 0;
  string mnm;
  string temp_regs[2] = {"%rax", "%rdx"};
  bool is_ref = false, is_mem[3] = {false}, isDiv = false;
  bool get_src1 = true, get_src2 = true, get_dst = true;
  bool isNeg = false, isAsg = false;
  string cmt = comment;
  EOperation op = i->GetOperation();

  string operand;

  switch (op) {
    case opAdd: mnm = "addq"; break;
    case opSub: mnm = "subq"; break;
    case opAnd: mnm = "andq"; break;
    case opOr:  mnm = "orq";  break;
    case opMul: mnm = "imulq"; break;
    case opDiv: mnm = "idivq";
                isDiv = true; break;
    case opNeg:
    case opNot: mnm = (op == opNeg ? "negq" : "notq");
                get_src2 = false;
                isNeg = true;
                break;
    case opAssign:
                get_src2 = false;
                isAsg = true;
                break;
    case opGoto:
                get_src2 = false;
                break;
  }

  /*
   *  get operands and destination
   *  if destination is in memory, use a temporary register
   *  also if operation is division or neg/not, src1 should go into a register(%rax)
   *  이 코드를 짠 사람을 죽이고 싶다고요? 저도 그렇습니다..
  */

  string src1, src2, dst, old_dst;
  if(get_src1) {
    src1 = Operand(i->GetSrc(1), &is_ref, &is_mem[0]);
    
    if(is_ref) {
      // memory면 한번 주소 레지스터로 가져와야함
      if(is_mem[0]) {
        Load(src1, temp_regs[reg_count], cmt, 8); // 8 is for pointer size
        cmt = "";
      }
      // move address to reg
      Load("("+src1+")", temp_regs[reg_count], cmt, 8);
      cmt = "";
      src1 = temp_regs[reg_count++]; //size will be 8
      is_mem[0] = false;
    } else if(isDiv || isNeg) {
      Load(src1, temp_regs[reg_count], cmt, OperandSize(i->GetSrc(1)));
      cmt = "";
      src1 = temp_regs[reg_count++];
      is_mem[0] = false;
    }
  }
  // for op which need only one operand, do not execute it
  if(get_src2) {
    is_ref = false;
    src2 = Operand(i->GetSrc(2), &is_ref, &is_mem[1]);

    if(is_ref) {
      if(is_mem[1]) {
        Load(src2, temp_regs[reg_count], cmt, OperandSize(i->GetSrc(2)));
        cmt = "";
      }
      Load("("+src2+")", temp_regs[reg_count], cmt, OperandSize(i->GetSrc(2)));
      cmt = "";
      src2 = temp_regs[reg_count++]; //size will be 8
      is_mem[1] = false;
    }
  }
  bool storeDest = false;
  if(get_dst) {
    is_ref = false;
    dst = Operand(i->GetDest(), &is_ref, &is_mem[2]);

    if(is_mem[2] && !isNeg && !isDiv) {
      // register를 dest로 사용해야함
      // case which src1, src2 are both reference not exists
      // for Neg, src1 is always load to reg
      old_dst = dst;
      dst = temp_regs[reg_count++];
      storeDest = true;
    }
  }

  /* Debugging
     for(int i=0; i<3; i++){
     if(is_mem[i])
     cmt = cmt + "1";
     else
     cmt = cmt + "0";
     }
  */
  switch (op) {
    case opAdd: 
    case opSub: 
    case opAnd: 
    case opOr:  
    case opMul:
      Load(src1, dst, cmt, OperandSize(i->GetSrc(1)));
      cmt = "";
      EmitInstruction(mnm, src2 + ", " + dst, cmt);
      break;
    case opDiv:
      EmitInstruction("cqo");
      EmitInstruction(mnm, src2);
      cmt = "";
      //dst = getRegister(dst, OperandSize(i->GetDest()));
      src1 = getRegister(src1, OperandSize(i->GetSrc(1)));
      //Store(src1, dst, cmt, OperandSize(i->GetSrc(1)));
      Store(src1, dst, cmt, OperandSize(i->GetSrc(1)));
      return;
    case opNeg:
    case opNot:
      // convert dst with size
      // string tempDst = getRegister(dst, OperandSize(i->GetDest()));
      // Load(src1, dst, cmt, OperandSize(i->GetSrc(1)));
      EmitInstruction(mnm, src1, cmt);
      cmt = "";
      src1 = getRegister(src1, OperandSize(i->GetSrc(1)));
      if(is_mem[2]==false)
        dst = getRegister(dst, OperandSize(i->GetDest()));
      Store(src1, dst, cmt, OperandSize(i->GetSrc(1)));
      return;
    case opAssign:
      if(is_mem[0]){
        Load(src1, dst, cmt, OperandSize(i->GetSrc(1)));
        cmt = "";
      } else {
        Store(src1, old_dst, cmt, OperandSize(i->GetSrc(1)));
        cmt = "";
        return; // to avoid doing Store into old_dst
      }
      break;
    case opAddress:
      EmitInstruction("leaq", src1 + ", " + dst, cmt);
      cmt = "";
      break;
    case opGoto:
      EmitInstruction("jmp", dst, cmt);
      cmt = "";
      return;
    // conditional branching
    // if src1 relOp src2 then goto dst
    case opEqual:
    case opNotEqual:
    case opLessThan:
    case opLessEqual:
    case opBiggerThan:
    case opBiggerEqual:
      EmitInstruction("cmpq", src2 + ", " + src1, cmt);
      cmt = "";
      EmitInstruction("j" + Condition(op), dst);
      return;
  }
  if(is_mem[2]){
    dst = getRegister(dst, OperandSize(i->GetDest()));
    Store(dst, old_dst, cmt, OperandSize(i->GetDest()));
  }
  /* TODO::src, dst reg가 같을 때 바꾸기
     if(src1.strcmp(dst) == 0)
     EmitInstruction(mnm, src2 + ", " + dst, comment)
     }
     else if((op != opSub) && src2.strcmp(dst)){
     }
     */

}

void CBackendx86_64::EmitInstruction(CTacInstr *i)
{
  assert(i != NULL);

  ostringstream cmt;
  string mnm;
  cmt << i;

  EOperation op = i->GetOperation();

  switch (op) {
    // binary operators
    case opAdd:
    case opSub:
    case opAnd:
    case opOr:
    case opMul:
    case opDiv:
    case opNeg:
    case opNot:
    case opAssign:
    case opAddress:
    case opGoto:
    case opEqual:
    case opNotEqual:
    case opLessThan:
    case opLessEqual:
    case opBiggerThan:
    case opBiggerEqual:
      EmitOperation(i, cmt.str());
      break;
      /*
      // memory operations
      // dst = src1
      case opAssign:
      Load(i->GetSrc(1), "%rax", cmt.str());
      Store(i->GetDest(), 'a');
      break;

      // pointer operations
      // dst = &src1
    case opAddress:
      EmitInstruction("leaq", Operand(i->GetSrc(1)) + ", %rax", cmt.str());
      EmitInstruction("movq", "%rax, " + Operand(i->GetDest()));
      break;
      */
      // dst = *src1
    case opDeref:
      // opDeref are not generated for now
      EmitInstruction("# opDeref", "not implemented", cmt.str());
      break;

    /*
      // unconditional branching
      // goto dst
    case opGoto:
      EmitInstruction("jmp", Operand(i->GetDest()), cmt.str());
      break;

      // conditional branching
      // if src1 relOp src2 then goto dst
    case opEqual:
    case opNotEqual:
    case opLessThan:
    case opLessEqual:
    case opBiggerThan:
    case opBiggerEqual:
      Load(i->GetSrc(1), "%rax", cmt.str());
      Load(i->GetSrc(2), "%rbx");
      EmitInstruction("cmpq", "%rbx, %rax");
      EmitInstruction("j" + Condition(op), Operand(i->GetDest()));
      break;


    */
      // function call-related operations
    case opTailCall:
      {
        isTailCall = true;
        EmitEpilogue();
        EmitInstruction("jmp", Operand(i->GetSrc(1)), cmt.str());
        break;
      }

    case opCall:
      {
        EmitInstruction("call", Operand(i->GetSrc(1)), cmt.str());

        // fix stack pointer
        CTacName *n = dynamic_cast<CTacName*>(i->GetSrc(1));
        assert(n != NULL);
        int npar = dynamic_cast<const CSymProc*>(n->GetSymbol())->GetNParams();
        //modified
        if (npar > 6) EmitInstruction("addq", Imm((npar-6)*8) + ", %rsp");

        // function result
        CTacTemp *t = dynamic_cast<CTacTemp*>(i->GetDest());
        if (t != NULL) {
          Store(i->GetDest(), 'a');
        }
      }
      break;

    case opReturn:
      if (i->GetSrc(1) != NULL) {
        Load(i->GetSrc(1), "%rax", cmt.str());
      }
      EmitInstruction("jmp", Label("exit"));
      break;

    case opParam:
      {
        CTacConst *t = dynamic_cast<CTacConst*>(i->GetDest());
        int paramIndex = t->GetValue();
        if(paramIndex > 6) {
          Load(i->GetSrc(1), "%rax", cmt.str());
          EmitInstruction("pushq", "%rax");
        }
        else {
          Load(i->GetSrc(1), param_regs[paramIndex-1], cmt.str());
        }
      } //EmitInstruction("pushq", "%rax");
      break;

      // special
    case opLabel:
      _out << Label(dynamic_cast<CTacLabel_prime*>(i)) << ":" << endl;
      break;

    case opNop:
      EmitInstruction("nop", "", cmt.str());
      break;

    case opDIM:
      {
        Load(i->GetSrc(1), "%rax", cmt.str());
        Load(i->GetSrc(2), "%rbx");
        EmitInstruction("movl", "(%rax, %rbx, 4), %eax");
        Store(i->GetDest(), 'a');
        break;
      }

    case opDOFS:
      {
        string s = ".done_" + to_string(tmp_label_count++);
        Load(i->GetSrc(1), "%rax", cmt.str());

        EmitInstruction("movl", "(%rax), %eax");
        EmitInstruction("leal", "4(,%eax,4), %eax");
        EmitInstruction("testl", "$4, %eax");
        EmitInstruction("jz", s);
        EmitInstruction("addl", "$4, %eax");
        _out << s << ":" << endl;

        Store(i->GetDest(), 'a');
        break;
      }

    default:
      EmitInstruction("# ???", "not implemented", cmt.str());
  }
}

void CBackendx86_64::EmitInstruction(string mnemonic, string args, string comment)
{
  _out << left
    << _ind
    << setw(7) << mnemonic << " "
    << setw(23) << args;
  if (comment != "") _out << " # " << comment;
  _out << endl;
}

void CBackendx86_64::Load(string src, string dst, string comment, int size)
{
  string mnm = "mov";
  string mod = "";

  // set operator modifier based on the operand size
  switch (size) {
    case 1: mod = "zbq"; break;
    case 2: mod = "zwq"; break;
    case 4: mod = "l"; dst = getRegister(dst, 4); break;
    case 8: mod = "q"; break;
  }

//  dst = getRegister(dst, size);

  // emit the load instruction
  EmitInstruction(mnm + mod, src + ", " + dst, comment);
}


void CBackendx86_64::Load(CTacAddr *src, string dst, string comment)
{
  assert(src != NULL);

  string mnm = "mov";
  string mod = "";

  // set operator modifier based on the operand size
  switch (OperandSize(src)) {
    case 1: mod = "zbq"; break;
    case 2: mod = "zwq"; break;
    case 4: mod = "l"; if(dst.substr(1,1).compare("r")==0) dst.replace(1, 1, "e"); break;
    case 8: mod = "q"; break;
  }

  // emit the load instruction
  EmitInstruction(mnm + mod, Operand(src) + ", " + dst, comment);
}
void CBackendx86_64::Store(string src, string dst, string comment, int size)
{
  string mod = "q";
  
  switch (size) {
    case 1: mod = "b"; break;
    case 2: mod = "w"; break;
    case 4: mod = "l"; break;
    case 8: mod = "q"; break;
  }

  EmitInstruction("mov" + mod, src + ", " + dst, comment);
}

void CBackendx86_64::Store(CTac *dst, char src_base, string comment)
{
  assert(dst != NULL);

  string mnm = "mov";
  string mod = "q";
  string src = "%";

  // compose the source register name based on the operand size
  switch (OperandSize(dst)) {
    case 1: mod = "b"; src += string(1, src_base) + "l"; break;
    case 2: mod = "w"; src += string(1, src_base) + "x"; break;
    case 4: mod = "l"; src += "e" + string(1, src_base) + "x"; break;
    case 8: mod = "q"; src += "r" + string(1, src_base) + "x"; break;
  }

  // emit the store instruction
  EmitInstruction(mnm + mod, src + ", " + Operand(dst), comment);
}

string CBackendx86_64::Operand(const CTac *op)
{
  const CTacName *n;
  const CTacConst *c;
  const CTacLabel_prime *l;

  string operand;

  if ((c = dynamic_cast<const CTacConst*>(op)) != NULL) {
    operand = Imm(c->GetValue());
  }
  else if ((n = dynamic_cast<const CTacName*>(op)) != NULL) {
    const CSymbol *s = n->GetSymbol();

    switch (s->GetSymbolType()) {
      case stGlobal:
      case stProcedure:
        operand = s->GetName();
        break;

      case stLocal:
      case stParam:
        {
          ostringstream o;
          if(s->isInReg()){
            o << s->GetBaseRegister();
          }
          else{
            o << s->GetOffset() << "(" << "%rbp" << ")";
          }
          operand = o.str();
        }
        break;
    }

    if (dynamic_cast<const CTacReference*>(n) != NULL) {
      Load(operand, "%rax", "", 8);
      operand = "(%rax)";
    }

  }
  else if ((l = dynamic_cast<const CTacLabel_prime*>(op)) != NULL) {
    operand = Label(l);
  } else {
    operand = "?";
  }

  return operand;
}
string CBackendx86_64::Operand(const CTac *op, bool* is_ref, bool* is_mem)
{
  const CTacName *n;
  const CTacConst *c;
  const CTacLabel_prime *l;

  string operand;

  if ((c = dynamic_cast<const CTacConst*>(op)) != NULL) {
    operand = Imm(c->GetValue());
  } else
    if ((n = dynamic_cast<const CTacName*>(op)) != NULL) {
      const CSymbol *s = n->GetSymbol();

      switch (s->GetSymbolType()) {
        case stGlobal:
        case stProcedure:
          operand = s->GetName();
          break;

        case stLocal:
        case stParam:
          {
            ostringstream o;
            if(s->isInReg()){
              o << s->GetBaseRegister();
            }
            else{
              o << s->GetOffset() << "(" << "%rbp" << ")";
              *is_mem = true;
            }
            operand = o.str();
          }
          break;
      }

      if (dynamic_cast<const CTacReference*>(n) != NULL) {
        *is_ref = true;
        //operand = "("+temp_reg")";
      }

    } else if ((l = dynamic_cast<const CTacLabel_prime*>(op)) != NULL) {
      operand = Label(l);
    } else {
      operand = "?";
    }

  return operand;
}

string CBackendx86_64::Imm(int value) const
{
  ostringstream o;
  o << "$" << dec << value;
  return o.str();
}

string CBackendx86_64::Label(const CTacLabel_prime* label) const
{
  CScope *cs = GetScope();
  assert(cs != NULL);

  ostringstream o;
  o << "l_" << cs->GetName() << "_" << label->GetLabel();
  return o.str();
  return "l_" + cs->GetName() + "_" + label->GetLabel();
}

string CBackendx86_64::Label(string label) const
{
  CScope *cs = GetScope();
  assert(cs != NULL);

  return "l_" + cs->GetName() + "_" + label;
}

string CBackendx86_64::Condition(EOperation cond) const
{
  switch (cond) {
    case opEqual:       return "e";
    case opNotEqual:    return "ne";
    case opLessThan:    return "l";
    case opLessEqual:   return "le";
    case opBiggerThan:  return "g";
    case opBiggerEqual: return "ge";
    default:            assert(false); break;
  }
}

int CBackendx86_64::OperandSize(CTac *t) const
{
  int size = 4;

  CTacName *name = dynamic_cast<CTacName*>(t);
  if (name != NULL) {
    const CType *t;

    // for references, we need the type of the original (referred to) symbol,
    // otherwise the type of the symbol itself
    CTacReference *ref = dynamic_cast<CTacReference*>(name);
    if (ref != NULL) {
      t = ref->GetDerefSymbol()->GetDataType();
      // additionally, if the reference is a pointer, we have to
      // get the base type. Example code pattern:
      // t0 <- &array
      // t1 <- array offset
      // t2 <- t0 + t1
      // *use of t2*: (... <- @t2 or if @t2 = ... goto ...)
      if (t->IsPointer()) {
        t = dynamic_cast<const CPointerType*>(t)->GetBaseType();
      }
    } else {
      t = name->GetSymbol()->GetDataType();
      // note: we do not want to dereference pointers here.
      // Example code pattern:
      // t0 <- &array
      // param 0 <- t0
    }

    assert(t != NULL);

    // for array types, we need the base type of the array
    if (t->IsArray()) {
      //cout << "INFO: array source detected: " << src << endl;
      t = dynamic_cast<const CArrayType*>(t)->GetBaseType();
    }

    size = GetSize_prime(t);
  }

  return size;
}

void CBackendx86_64::StackDump(CSymtab *symtab)
{
  assert(symtab != NULL);
  vector<CSymbol*> slist = symtab->GetSymbols();

  // dump
  _out << _ind << "# stack offsets:" << endl;
  for (size_t i=0; i<slist.size(); i++) {
    CSymbol *s = slist[i];
    ESymbolType st = s->GetSymbolType();

    if ((st == stLocal) || (st == stParam)) {
      ostringstream loc;
      if(s->isInReg()){
        loc << right << setw(4) << s->GetBaseRegister();
        // << "(" << "" << ")";
        _out << _ind << "#   "
          << left << setw(10) << loc.str() << "  "
          << right << setw(2) << GetSize_prime(s->GetDataType()) << "  "
          << s
          << endl;

      }
      else{
        loc << right << setw(4) << s->GetOffset()
          << "(" << "%rbp" << ")";
        _out << _ind << "#   "
          << left << setw(10) << loc.str() << "  "
          << right << setw(2) << GetSize_prime(s->GetDataType()) << "  "
          << s
          << endl;

      }

    }
  }
  _out << endl;

}

void CBackendx86_64::EmitCalleePush(const boost::dynamic_bitset<> used_regs){
  /// bit order: (low) rbx, r12, r13, r14, r15 (high)
  for(int i=0; i<5; i++)
    if(used_regs[i])
      EmitInstruction("pushq", callee_regs[i], "save callee saved registers");
}

void CBackendx86_64::EmitCalleePop(const boost::dynamic_bitset<> used_regs){
  /// bit order: (low) rbx, r12, r13, r13, r15 (high)
  for(int i=4; i>=0; i--)
    if(used_regs[i])
      EmitInstruction("popq", callee_regs[i], "restore callee saved registers");
}

void CBackendx86_64::EmitCallerPush(const boost::dynamic_bitset<> used_regs){
  for(int i=0; i<2; i++)
    if(used_regs[i])
      EmitInstruction("pushq", caller_regs[i],"save caller saved registers");
}
void CBackendx86_64::EmitCallerPop(const boost::dynamic_bitset<> used_regs){
  for(int i=1; i>=0; i--)
    if(used_regs[i])
      EmitInstruction("popq", caller_regs[i],"restore caller saved registers");
}

//TODO: fix it after register coloring
void CBackendx86_64::EmitParamPush(int param_num){
  for(int i = 0; i < param_num; i++)
    EmitInstruction("pushq", param_regs[i], "put param regs into stack to use");
}
