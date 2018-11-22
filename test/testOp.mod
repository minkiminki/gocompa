//
// testOp
//
// test operations
//

module testOp;
var a : integer;
		b : integer;
		c : integer;
    d : boolean;
    e : boolean;

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

  if(c = 1)
    then d := true
    else d := false
  end;

  WriteStr("(test1) c := a+b\n");
	c := a+b;
  WriteInt(c); WriteLn();

  WriteStr("(test2) c := a-b\n");
	c := a-b;
  WriteInt(c); WriteLn();

  WriteStr("(test3) c := a*b\n");
	c := a*b;
  WriteInt(c); WriteLn();

  WriteStr("(test4) c := a/b\n");
	c := a/b;
  WriteInt(c); WriteLn();

  WriteStr("(test5) c := -a\n");
	c := -a;
  WriteInt(c); WriteLn();

  WriteStr("(test6) e := !d, if(e) 1 else 0 \n");
	e := !d;
  if(e) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test7) e := d || false, if(e) 1 else 0 \n");
  e := d || false;
  if(e) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test8) e := d && false, if(3) 1 else 0 \n");
  e := d && false;
  if(e) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test9) e := a = b, if(e) 1 else 0 \n");
  if(a = b) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test10) if(a > b) 1 else 0 \n");
  if(a > b) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test10) if(a >= b) 1 else 0 \n");
  if(a >= b) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test10) if(a < b) 1 else 0 \n");
  if(a < b) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test10) if(a <= b) 1 else 0 \n");
  if(a <= b) then WriteInt(1) else WriteInt(0) end;
  WriteLn()

end testOp.
