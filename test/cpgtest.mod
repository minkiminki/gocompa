//
// test08
//
// local integer arrays
//
// expected output: 1987654321 (no newline)
//

module cpgtest;

procedure test();
var a : integer;
    b : integer;
    c : integer;
begin
  a := 4 * (8 - 7); // 4
  b := a - (a + 1); // -1
  c := b + a; // 3
  WriteInt(c - a) // -1
end test;

begin
  test()
end cpgtest.
