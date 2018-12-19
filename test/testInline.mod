//
// testInline
//
// test inlining
//

module testInline;
var a : integer;

function foo(i: integer): integer;
begin
  if(i>5) then return i
  else return foo(i+1)+1
  end
end foo;

function ReadNumber(str: char[]): integer;
var i: integer;
begin
WriteStr(str);
  i := ReadInt();
    return i
    end ReadNumber;

begin
  a := ReadNumber("a: ");

  WriteInt(foo(a)); WriteLn()

end testInline.
