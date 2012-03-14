program sample7;

type int_ptr = ^integer;

var index   : integer;
    myarray : array[1..5] of integer;
    range : 1 .. 10;
    ptr : int_ptr;
begin
   
   for index := 1 to 5 do
          myarray[index]:= index;

   for index := 1 to 5 do
          write(myarray[index]);
   
   range := 999;
   
   range := ptr^;
   index := myarray[5] + 5;
   
end.
