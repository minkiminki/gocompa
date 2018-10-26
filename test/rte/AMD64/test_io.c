//------------------------------------------------------------------------------
/// @brief test cases for IA32 IO.s
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2012/10/12 Bernhard Egger created
/// 2018/09/01 Bernhard Egger adapted to new IO functions
///
/// @section license_section License
#// Copyright (c) 2012-2018, Computer Systems and Platforms Laboratory, SNU
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
#include <string.h>
#include <limits.h>

#include "snupl.h"

void testWriteInt(int v)
{
  printf("    %12d:    ", v);
  fflush(stdout);
  WriteInt(v);
  printf("\n");
}

void testWriteStr(const char *s)
{
  size_t len = strlen(s)+1; // strlen does not count the terminating \0
  void *snuplstr = createSnuPLArray(s, sizeof(char), 1, len);

  printf("    '%s':\n", s);
  printf("      '"); fflush(stdout);
  WriteStr(snuplstr);
  printf("'\n");
}

void testWriteChar(char c)
{
  printf("    '%c':       '", c);
  fflush(stdout);
  WriteChar(c);
  printf("'\n");
}

void testWriteLn(void)
{
  printf("    there should be an empty line in between this...\n");
  WriteLn();
  printf("    ...and this line\n");
}

void testOutput(void)
{
  printf("Testing SnuPL/1 output functions\n");
  printf("\n");

  // WriteInt
  printf("  WriteInt\n");
  testWriteInt(0);
  testWriteInt(1);
  testWriteInt(-1);
  testWriteInt(INT_MAX);
  testWriteInt(INT_MIN);
  for (int i=0; i<10; i++) {
    testWriteInt((int)(rand() - RAND_MAX/2));
  }
  printf("\n");

  // WriteStr
  printf("  WriteStr\n");
  testWriteStr("Hello, world");
  testWriteStr("");
  testWriteStr("A");
  testWriteStr("With newline\n");
  printf("\n");

  // WriteChar
  printf("  WriteChar\n");
  testWriteChar('A');
  testWriteChar('a');
  testWriteChar('b');
  testWriteChar(0x44);
  printf("\n");

  // WriteLn
  printf("  WriteLn\n");
  testWriteLn();

  printf("  Done.\n");
  printf("\n");
}

void testInput(void)
{
  int i, v;

  printf("Testing SnuPL/1 input functions\n");
  printf("\n");

  // ReadInt
  printf("  ReadInt: enter 10 integer numbers\n");

  for (i=0; i<10; i++) {
    printf("    enter number: "); fflush(stdout);
    v = ReadInt();
    printf("    got %d\n", v);
  }
  printf("  Done.\n");
  printf("\n");
}


void main(int argc, char **argv)
{
  testOutput();
  testInput();
}
