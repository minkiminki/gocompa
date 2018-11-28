//
// fibonacci
//
// fibonacci computation
//
// without tco, stackoverflow will occurs for 1000000
//

module idk;

var i : integer;

function fib_helper4(n: integer; prev_2: integer; prev_1: integer): integer;
begin
  WriteLn();return n
end fib_helper4;

function fib_helper1(n: integer; prev_2: integer; prev_1: integer): integer;
begin
  WriteInt(n); return fib_helper4(n, prev_1, prev_2)
//    return fib_helper2(n-1, prev_1, prev_1 + prev_2)
end fib_helper1;

begin
  WriteInt(fib_helper1(5, 1, 1))
end idk.
