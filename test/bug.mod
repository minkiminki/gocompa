//
// test02
//
// computations with integers arrays
// also test alignment of global data
//
// expected output: 1987654321 (no newline)
//

module bug;

var a : integer[10];
    i : integer;

begin
  i := 0;
  while (i < 5) do
	a[i] := i;
  WriteInt(a[i]);
    i := i+1
  end;
  i := 0;
  while  (i < 5) do
	WriteInt(a[i]);
    i := i+1
  end

end bug.
