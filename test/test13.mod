//
// test01
//
// simple computations with integers
//
// expected output: 710-3 (no newline)
//

module test01;

var a : integer;
    b : integer;
    c : integer;

begin
  a := 2;
  b := 5;

  c := a+b;
  WriteInt(c);

	if(a > 1) then c := a*b
	else c := a-b
	end;
  WriteInt(c);

  c := -b+a;
  WriteInt(c)
end test01.
