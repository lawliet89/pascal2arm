program sample7;

var index   : integer;
    myarray : array[1..5] of integer;
    
begin
   
   for index := 1 to 5 do
          myarray[index]:= index;

   for index := 1 to 5 do
          write(myarray[index]);
   
end.
