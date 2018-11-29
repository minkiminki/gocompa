//
// testMul
//
// test Mul operations
//

module testMul;
var a : integer;
		b : integer;
		c : integer;
		d : integer[2];

function ReadNumber(str: char[]): integer;
var i: integer;
begin
WriteStr(str);
  i := ReadInt();
    return i
    end ReadNumber;

begin
  a := ReadNumber("a: ");
  b := ReadNumber("b: ");

  WriteStr("(test1) c := a/b\n");
	c := a/b;
  WriteInt(c); WriteLn();

  WriteStr("(test2) d[0] := a/b\n");
	d[0] := a/b;
  WriteInt(d[0]); WriteLn();

  WriteStr("(test3) d[1] := b\n");
	d[1] := b;
  WriteInt(d[1]); WriteLn();

  WriteStr("(test4) d[0] := d[0] / d[1]\n");
	d[0] := d[0] / d[1];
  WriteInt(d[0]); WriteLn();

  WriteStr("(test5) a := d[0] / b\n");
	a := d[0] / b;
  WriteInt(a); WriteLn();

  WriteStr("(test6) a := b / d[1]\n");
	a := b / d[1];
  WriteInt(a); WriteLn()

end testMul.
