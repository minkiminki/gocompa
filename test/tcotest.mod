//
// tailcall
//
// test for tail call optimization
//

module tcotest;

var n: integer[2];

// f(a: integer[2]): integer
// tco should fail
function f(a: integer[2]): integer;
var c, d: integer;
    e: integer[2];
begin
  while (c # d) do
    if (c > d) then c := c - d
    else d := d - c
    end
  end;
  return f(e) // not tail call : using caller's stack
end f;

// g(a: integer[2]): integer
// tco should success
function g(a: integer[2]): integer;
var c, d: integer;
begin
  while (c # d) do
    if (c > d) then c := c - d
    else d := d - c
    end
  end;
  return g(a) // tail call
end g;

// f(a: integer[2]): integer
// tco should success
function h(a: integer[2]): integer;
var c, d: integer;
begin
  while (c # d) do
    if (c > d) then c := c - d
    else d := d - c
    end
  end;
  return h(n) // tail call
end h;

begin
  WriteInt(h(n))
end tcotest.
