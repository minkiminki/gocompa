//
// testOp
//
// test operations
//

module testOp;
var a : integer;
		b : integer;
		c : integer;

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
  c := ReadNumber("d(bool 1/0): ");

  WriteStr("(test1) c := a+b\n");
	c := a+b;
  WriteInt(c); WriteLn()

end testOp.
