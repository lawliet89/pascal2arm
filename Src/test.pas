program sample6;

var A, B, C, D, E :  integer;

procedure hello();
   begin
	write('A');
   end;
function incr(var X: integer):integer;
   begin
       incr := X + 1;
       X := 5;
   end;
begin
   A:=1;
   B:=2;
   C:=3;
   D:=4;
   E:=5;
   for E := 1 to 5 do
	B:=incr(A);
   write(B);
   hello();
end.
