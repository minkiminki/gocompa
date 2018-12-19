//
// test17
//
// test add
//

module test17;
var d : boolean;
    e : boolean;

begin
  d := true;
  e := d || false;
  if(e)
    then WriteInt(1)
    else WriteInt(0)
  end
end test17.
