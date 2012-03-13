program sample8;

type int_pointer =  ^integer;

var  mypointer :int_pointer;

begin
   new(mypointer);
   mypointer^ := 10;
   write(mypointer^);
   dispose(mypointer);
end.

