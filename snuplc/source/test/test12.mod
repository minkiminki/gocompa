//
// test12
//
// function return values
//
// expected output: returned values are stored according to 
// their size
//

module test12;

function Int():integer;
var i: integer;
begin
  i := 75;
  return i
end Int;

function Char(): char;
var c: char;
begin
  c := 'B';
  return c
end Char;

function Bool(): boolean;
var b: boolean;
begin
  b := true;
  return b
end Bool;

procedure Test();
var b: boolean;
    c: char;
    i: integer;
begin
  b := Bool();
  c := Char();
  i := Int()
end Test;

begin
  Test()
end test12.
