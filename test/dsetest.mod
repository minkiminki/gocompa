//
// dsetest
//
// test for dead store elimination
//

module dsetest;

function f(): integer;
var a, b, c, d, e: integer;
begin
  a := 1 * 6;
  b := a - 1 * a;

  if(b > 4) then c := a + b
  else a := b + a; c := a - b
  end;

  d := a + b + c;
  while(c > 4) do d := d + 1
  end;

  e := a + b + c + d;
  return 5
end f;

begin
  f()
end dsetest.
