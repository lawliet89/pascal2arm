program sample6;

var A, B, C, D, E :  integer;

function incr(X: integer):integer;
   begin
       incr := X + 1;
   end;

begin
   A:=1;
   B:=2;
   C:=3;
   D:=4;
   E:=5;
   B:=incr(A);
   write(B);
end.
