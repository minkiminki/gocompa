//------------------------------------------------------------------------------
/// @brief SnuPL C language definitions (AMD64)
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "snupl.h"

//
// create an SnuPL array
//
// see snupl.h for details
//
void* createSnuPLArray(const void *data, size_t elem_size, int dim, ...)
{
  va_list ap;
  struct {
    int dim;
    int d[];
  } __attribute__ ((aligned (8))) *header;
  void *array;
  size_t data_size, offset;

  // run through all dimensions and calculate total size of data
  va_start(ap, dim);
  data_size = (size_t)elem_size;
  for (int i=0; i<dim; i++) {
    data_size *= va_arg(ap, int);
  }
  va_end(ap);

  // allocate memory
  offset = 1+dim;
  if (offset & 1) offset++; // align to 8-byte boundaries on x86_64
  offset = offset*sizeof(int);

  array = malloc(offset+data_size);

  // create SnuPL/1 array header
  header = array;
  header->dim = dim;
  va_start(ap, dim);
  for (int i=0; i<dim; i++) {
    int d = va_arg(ap, int);
    header->d[i] = d;
  }
  va_end(ap);

  // copy data
  memcpy(array + offset, data, data_size);

  return array;
}
