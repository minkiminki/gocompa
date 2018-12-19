//
// testOp
//
// test operations
//

module testOp;
var a : integer;
		b : integer;
    c : boolean;
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
  WriteStr("type only 0 or 1\n");
  a := ReadNumber("bool a: ");
  b := ReadNumber("bool b: ");

  if(a = 1)
    then c := true
    else c := false
  end;

  if(b = 1)
    then d := true
    else d := false
  end;

  WriteStr("(test1) if(a) 1\n");
  if(c)
    then WriteInt(1)
  end;
  WriteLn();

  WriteStr("(test2) if(a=b) 1 else 0\n");
  if(c=d)
    then a:= 1
    else a:= 0
  end;
  WriteInt(a); WriteLn();

  WriteStr("(test3) e := !b, if(e) 1 else 0 \n");
  e := !d;
  if(e) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test4) e := b || false, if(e) 1 else 0 \n");
  e := d || false;
  if(e) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test5) e := b && false, if(3) 1 else 0 \n");
  e := d && false;
  if(e) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test6) e := a = b, if(e) 1 else 0 \n");
  if(a = b) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test7) if(a > b) 1 else 0 \n");
  if(a > b) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test8) if(a >= b) 1 else 0 \n");
  if(a >= b) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test9) if(a < b) 1 else 0 \n");
  if(a < b) then WriteInt(1) else WriteInt(0) end;
  WriteLn();

  WriteStr("(test10) if(a <= b) 1 else 0 \n");
  if(a <= b) then WriteInt(1) else WriteInt(0) end;
  WriteLn()

end testOp.
