#-------------------------------------------------------------------------------
#// @brief SnuPL I/O library (x86_64)
#// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
#// @section changelog Change Log
#// 2018/09/01 Bernhard Egger created
#//
#// @section license_section License
#// Copyright (c) 2018, Computer Systems and Platforms Laboratory, SNU
#// All rights reserved.
#//
#// Redistribution and use in source and binary forms,  with or without modifi-
#// cation, are permitted provided that the following conditions are met:
#//
#// - Redistributions of source code must retain the above copyright notice,
#//   this list of conditions and the following disclaimer.
#// - Redistributions in binary form must reproduce the above copyright notice,
#//   this list of conditions and the following disclaimer in the documentation
#//   and/or other materials provided with the distribution.
#//
#// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
#// IMPLIED WARRANTIES OF MERCHANTABILITY  AND FITNESS FOR A PARTICULAR PURPOSE
#// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER  OR CONTRIBUTORS BE
#// LIABLE FOR ANY DIRECT,  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSE-
#// QUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF  SUBSTITUTE
#// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT
#// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY
#// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
#// DAMAGE.
#-------------------------------------------------------------------------------

  .text
  .align 8

.global ReadInt
.global WriteInt
.global WriteStr
.global WriteChar
.global WriteLn

.extern DOFS

#-------------------------------------------------------------------------------
# function ReadInt()
# (C: int ReadInt(void))
#
# reads a (decimal) value string from stdin and returns it a signed integer
#
# Reads strings in the format ["-"] digit {digit}.
# Recognizes newline, space and tab as number separators.
# Illegal and superfluous characters are skipped.
# Does not check for overflow.
#
# x86_64 Linux calling convention (result = %rax)
ReadInt:
  subq    $24, %rsp             # align at 32-byte boundary

  movq    %rsp, %r10            # pointer to input string starts at %rsp
                                # We need at least an 11-char buffer to store
                                # '-2147483648'
  movq    %rsp, %r11
  addq    $12, %r11             # address after last character of input buffer

.Lread:
  movq    $1, %rdx              # %rdx = number of characters to read
  movq    %r10, %rsi            # %rsi = pointer to buffer
  movq    $0, %rdi              # %rdi = stdin
  movq    $0, %rax              # %rax = 0 (read syscall)
  syscall                       # syscall

  testq   %rax, %rax            # end of input?
  je      .Lscan

  movb    (%rsi), %al           # load character into %al
  cmpb    $0xa, %al             # early-exit on newlines before increasing %ecx
  je      .Lskiploop

  incq    %r10
  cmpq    %r10, %r11            # buffer full?
  je      .Lskiploop

  cmpb    $0x2d, %al            # '-'?
  je      .Lread

  subb    $0x30, %al            # convert to number
  cmpb    $10, %al              # valid digit?
  jb      .Lread

  decq    %r10                  # character not valid -> discard

.Lskiploop:
  movb    (%rsi), %al           # get last read character into %al

  cmpb    $0xa, %al             # newline ?
  je      .Lscan
  cmpb    $0x9, %al             # tab?
  je      .Lscan
  cmpb    $0x20, %al            # space ?
  je      .Lscan

  movq    $1, %rdx              # %rdx = number of characters to read
  leaq    12(%rsp), %rsi        # %rsi = pointer to (scratch) buffer
  movq    $0, %rdi              # %rdi = stdin
  movq    $0, %rax              # %rax = 0 (read syscall)
  syscall                       # syscall

  testq   %rax, %rax            # end of input?
  jne     .Lskiploop

.Lscan:
  movb    $0, (%r10)            # terminate input buffer


  movq    $0, %rax              # accumulated number
  xorq    %r11, %r11            # negative flag
  movq    %rsp, %r10            # input string to parse

  movq    $10, %rdi             # base multiplicator

  cmpb    $0x2d, (%r10)         # first character a '-'?
  jne     .Lscanloop

  movq    $1, %r11              # set negative flag
  incq    %r10

.Lscanloop:
  movzbq  (%r10), %r9           # read character
  incq    %r10

  subq    $0x30, %r9            # conver to number
  cmpq    $9, %r9               # valid digit?
  ja      .Lscandone

  mulq    %rdi                  # new value = 10 * old value
  addq    %r9, %rax             #             + digit

  jmp     .Lscanloop

.Lscandone:
  testq   %r11, %r11            # negative?
  je      .Lexit

  negq    %rax                  # negate

.Lexit:
  addq    $24, %rsp
  retq


#-------------------------------------------------------------------------------
# procedure WriteInt(value)
# (C: void WriteInt(int value))
#
# prints a signed integer value in decimal notation to stdout
#
# x86_64 Linux calling convention (1st parameter in %edi on entry)
WriteInt:
  subq    $24, %rsp             # align at 32-byte boundary

  movq    $10, %r9              # base divident in %r9
  leaq    16(%rsp), %rsi        # pointer to result string in %rsi
                                # (decremented before use)

  movslq  %edi, %rax            # move value into %rax
  shrl    $31, %edi             # sign flag in %edi

  je      .Lloop                # if positive goto Lloop
  negq    %rax                  # otherwise negate value first

.Lloop:
  xorq    %rdx, %rdx            # since %rax is positive
  divq    %r9                   # divide value by base
  addq    $0x30, %rdx           # add '0' to remainder

  decq    %rsi
  movb    %dl, (%rsi)           # add digit to result string

  testq   %rax, %rax            # more digits?
  jne     .Lloop

  testl   %edi, %edi            # sign set?
  je      .Lprint

  decq    %rsi
  movb    $0x2d, (%rsi)         # output '-' sign

.Lprint:
  leaq    16(%rsp), %rdx
  subq    %rsi, %rdx            # %rdx = number of characters (= %rbp+16 - %rsi)
                                # %rsi = pointer to string (already set)
  movq    $1, %rdi              # %rdi = stdout
  movq    $1, %rax              # %rax = 1 (write syscall)
  syscall                       # syscall

  addq    $24, %rsp
  retq


#-------------------------------------------------------------------------------
# procedure WriteStr(str: char[])
# (C: void WriteStr(ptr to SnuPL/1 array of char str)
#
# prints string to stdout.
# Note: the str parameter is a pointer to an SnuPL/1 array!
#
# x86_64 Linux calling convention (1st parameter in %rdi on entry)
WriteStr:
  pushq   %rbx
  pushq   %rcx
  subq    $8, %rsp              # align at 32-byte boundary

  movq    %rdi, %rbx            # save pointer to string in %rbx

  # get start of string (array) data
                                # str argument (pointer to array) already in %rdi
  callq   DOFS                  # get offset into data array
  addq    %rax, %rbx            # add offset

  # determine string length
  movq    %rbx, %rdi            # pointer to beginning of string
  movq    $-1, %rcx             # max positive (unsigned) value
  xorq    %rax, %rax            # set %al = 0 (search for \0)
  cld                           # clear direction flag
  repne   scasb                 # find \0 in string
  notq    %rcx                  # ecx = max. value - string length (incl. \0)
  decq    %rcx                  # exclude \0

  # print string
  movq    %rcx, %rdx            # %rdx = number of characters
  movq    %rbx, %rsi            # %rsi = pointer to string
  movq    $1, %rdi              # %rdi = stdout
  movq    $1, %rax              # %rax = 1 (write syscall)
  syscall                       # syscall

  addq    $8, %rsp
  popq    %rcx
  popq    %rbx
  retq


#-------------------------------------------------------------------------------
# procedure WriteChar(c: char)
# (C: void WriteChar(char c))
#
# prints character c to stdout.
#
# x86_64 Linux calling convention (1st parameter in %edi on entry)
WriteChar:
  subq    $24, %rsp             # align at 32-byte boundary

  # prepare 'c' on stack
  movl    %edi, (%rsp)          # 'c'

  # print string
  movq    $1, %rdx              # %rdx = number of characters
  movq    %rsp, %rsi            # %rsi = pointer to string
  movq    $1, %rdi              # %rdi = stdout
  movq    $1, %rax              # %rax = 1 (write syscall)
  syscall                       # syscall

  addq    $24, %rsp
  retq


#-------------------------------------------------------------------------------
# procedure WriteLn
# (C: void WriteLn(void))
#
# prints a newline to stdout.
#
# x86_64 Linux calling convention
WriteLn:
  subq    $24, %rsp             # align at 32-byte boundary

  # prepare newline
  movb    $0x0a, (%rsp)         # 0x0a (newline)

  # print string
  movq    $1, %rdx              # %rdx = number of characters
  movq    %rsp, %rsi            # %rsi = pointer to string
  movq    $1, %rdi              # %rdi = stdout
  movq    $1, %rax              # %rax = 1 (write syscall)
  syscall                       # syscall

  addq    $24, %rsp
  retq
