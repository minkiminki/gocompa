#-------------------------------------------------------------------------------
#// @brief SnuPL dynamic array support library
#// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
#// @section changelog Change Log
#// 2016/03/24 Bernhard Egger created
#// 2016/04/16 Bernhard Egger ported from IA32 to AMD64
#//
#// @section license_section License
#// Copyright (c) 2016-2017, Computer Systems and Platforms Laboratory, SNU
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

.global DIM
.global DOFS

#-------------------------------------------------------------------------------
# Dynamic array implementation
#
# The code below assumes the following layout for arrays:
#
#   ----------------------------------------------
#   | #dim | d1 | d2 | ... | dn | [pad] | data ...
#   ----------------------------------------------
#   ^                                   ^
#   |                                   |
#   a                                align 8
#
# i.e., the number of dimensions (#dim) and the size of each dimension (d?) are
# stored as 4-byte integers in front of the actual data
#


#-------------------------------------------------------------------------------
# function DIM(a, d)
# (C: int DIM(void *a, int d)
#
# returns the size of the d-th dimension of array a (d >= 1)
# returns the number of dimensions of array a (d == 0)
#
# x86_64 Linux calling convention, leaf function (no stack frame)
DIM:
  movl    (%rdi, %rsi, 4), %eax # load dimension (no range checks)
  ret


#-------------------------------------------------------------------------------
# function DOFS(a)
# (C: int DOFS(void *a))
#
# returns the offset to the beginning of the data of an array a
#
# x86_64 Linux calling convention, leaf function (no stack frame)
DOFS:
  movl    (%rdi), %ecx          # load number of dimensions, #dim
  leal    4(,%ecx,4), %eax      # eax = 4 + 4*#dim
  testl   $1, %ecx              # #dim odd?
  jnz     .done                 # if yes, we're done
  addl    $4, %eax              # otherwise add 4 to align at 8
.done:
  ret
