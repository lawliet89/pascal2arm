program sample8;

type int_pointer =  ^integer;

var  
	ptr1, ptr2 :int_pointer;
	A : integer;

begin
   new(ptr1);
   ptr1^ := 10;
   ptr2 := ptr1;
   A := 10 + ptr1^;
   //write(ptr1^);
   dispose(ptr1);
end.

