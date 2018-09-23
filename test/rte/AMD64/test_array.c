//------------------------------------------------------------------------------
/// @brief test cases for AMD64 ARRAY.s
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2016/04/16 Bernhard Egger created
/// 2016/09/01 Bernhard Egger adapted to use createSnuPLArray
///
/// @section license_section License
#// Copyright (c) 2016-2018, Computer Systems and Platforms Laboratory, SNU
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
#include <stdio.h>

#include "snupl.h"

void dump(void *a)
{
  int i, dim, dofs, elem, size;

  dim = DIM(a, 0);
  dofs = DOFS(a);

  printf("dumping a (%p, %d dimensions, %d offset to data)\n", a, dim, dofs);

  elem = 1;
  i = 1;
  while (i <= dim) {
    int d = DIM(a, i);
    printf("  dim %d: %d\n", i, d);
    elem = elem * d;
    i++;
  }
  size = elem*sizeof(int);

  printf("  # elements: %d\n", elem);
  printf("  data size: %d\n", size);

  void *p = a + dofs, *end = a + dofs + size;
  printf("  data beginning at %p:\n", p);
  printf("  data ending    at %p:\n", end);
  while (p < end) {
    printf("  %d", *(int*)p);
    p += 4;
  }
  printf("\n");
}


int main(void)
{
  int data[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
  void *a;

  printf("1-dimensional integer array with 16 elements\n");
  a = createSnuPLArray(data, sizeof(int), 1, 16);
  dump(a);
  free(a);

  printf("2-dimensional integer array with 2x8 elements\n");
  a = createSnuPLArray(data, sizeof(int), 2, 2, 8);
  dump(a);
  free(a);

  printf("3-dimensional integer array with 2x2x4 elements\n");
  a = createSnuPLArray(data, sizeof(int), 3, 2, 2, 4);
  dump(a);
  free(a);

  printf("4-dimensional integer array with 2x2x2x2 elements\n");
  a = createSnuPLArray(data, sizeof(int), 4, 2, 2, 2, 2);
  dump(a);
  free(a);

  return 0;
}
