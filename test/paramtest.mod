//
// paramter test
//

module paramtest;
function process(n0: integer; n1: integer; n2: integer; n3: integer; n4: integer): integer;
begin
  WriteInt(n0);
  WriteInt(n1);
  WriteInt(n2);
  WriteInt(n3);
  WriteInt(n4)
end process;

begin
  process(ReadInt(), ReadInt(), ReadInt(), ReadInt(), ReadInt())
end paramtest.
