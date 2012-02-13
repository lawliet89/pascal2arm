program sample4;
var A, B :  integer;
begin
   B:=4;
   for A := 1 to 10 do
      begin
	 write(A);
	 B := B + 1;
      end;
   write(B);
end.
