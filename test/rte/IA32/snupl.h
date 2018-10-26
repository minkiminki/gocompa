//------------------------------------------------------------------------------
/// @brief SnuPL C language definitions
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2018/09/01 Bernhard Egger created
///
/// @section license_section License
/// Copyright (c) 2018, Computer Systems and Platforms Laboratory, SNU
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

#ifndef __SNUPL_H__
#define __SNUPL_H__

#include <stdarg.h>

//
// array functions defined in ARRAY.s
//
// argument 'a' must be a SnuPL array
//
int DIM(const void *a, int d);
int DOFS(const void *a);

//
// IO functions defined in IO.s
//
void WriteInt(const int v);
void WriteStr(const void *s);  // s must be a 1-dimensional SnuPL character array
void WriteChar(const char c);
void WriteLn(void);
int  ReadInt(void);

//
// create an SnuPL array
//
// this creates a COPY of the data, free the array with free() after usage
// (refer to ARRAY.s for details on the array format of SnuPL/1)
//
// usage:
// - create a 1-dimensional 'int' array with 10 elements
//   int data[10];
//   = createSnuPLArray(data, sizeof(int), 1, 10);
// - create a 3-dimensional 'char' array with dimensions 3x4x5
//   char data[3][4][5];
//   = createSnuPLArray(data, sizeof(char), 3, 3, 4, 5);
// - create a dynamic 3-dimensional 'int' array with dimensions 3x4x5
//   int *data = (int*)malloc(sizeof(int)*3*4*5);
//   = createSnuPLArray(data, sizeof(int), 3, 3, 4, 5);
//
// WARNING: SnuPL/1 defines the following data types and sizes
//   integer      4 bytes     (= int in C)
//   char         1 byte      (= char in C)
//   boolean      1 byte      (= char in C)
// if the data sizes differ in your C implementation, this code will break
//
void* createSnuPLArray(const void *data, size_t elem_size, int dim, ...);

#endif // __SNUPL_H__
