//
// fibonacci
//
// fibonacci computation
//
// without tco, stackoverflow will occurs for 1000000
//

module fibonacci_tco;

var i : integer;

function fib_helper(n: integer; prev_2: integer; prev_1: integer): integer;
begin
  if (n <= 1) then
    return prev_1
  else
    return fib_helper(n-1, prev_1, prev_1 + prev_2)
  end
end fib_helper;

function fib(n: integer): integer;
begin
  fib_helper(n, 1, 1)
end fib;

function ReadNumber(str: char[]): integer;
var i: integer;
begin
  WriteStr(str);
  i := ReadInt();
  return i
end ReadNumber;

begin
  WriteStr("Fibonacci numbers"); WriteLn();
  WriteLn();

  i := ReadNumber("Enter a number (0 to exit): ");

  while (i > 0) do
    WriteStr("fibonacci("); WriteInt(i); WriteStr(") = ");
    WriteInt(fib(i)); WriteLn();

    i := ReadNumber("Enter a number (0 to exit): ")
  end
end fibonacci_tco.
