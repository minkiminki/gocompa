//
// testMul
//
// test Mul operations
//

module testMul;
var d : integer[2];

function ReadNumber(str: char[]): integer;
var i: integer;
begin
WriteStr(str);
  i := ReadInt();
  return i
end ReadNumber;

begin
  d[0] := ReadNumber("d[0]: ");
  d[1] := ReadNumber("d[1]: ");

  WriteStr("(test4) d[0] := d[0] / d[1]\n");
	d[0] := d[0] / d[1];
  WriteInt(d[0]); WriteLn();

  WriteStr("(test4) d[0] := d[0] / 2\n");
	d[0] := d[0] / 2;
  WriteInt(d[0]); WriteLn()

end testMul.
