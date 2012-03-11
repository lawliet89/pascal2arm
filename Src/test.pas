program sample6;

var 
	A, B, C :  integer;
	bool : boolean;
function incr(X: integer):integer;
   begin
       incr := X + 1;
   end;

begin
   A:=1;
   B:=incr(A);
   write(B);
end.
