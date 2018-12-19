//
// paramter test
//

module paramtest;
function process(n0: integer; n1: integer; n2: integer; n3: integer): integer;
begin
  WriteInt(n0);
  WriteLn();
  WriteInt(n1);
  WriteLn();
  WriteInt(n2);
  WriteLn();
  WriteInt(n3);
  WriteLn()
end process;

begin
  process(1,2,3,4)
end paramtest.
