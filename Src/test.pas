PROGRAM HelloWorld;

VAR
	test, test2: integer = 100;
	float: real;
	character: char;
	bool: boolean;
	A,B, C : integer;

PROCEDURE testing;
	VAR
		test: boolean;
	BEGIN
		//test := 123;
	END;
	
BEGIN
	{*test := 5;		//Simple
	test2 := 6;
	test := 1+2;
	test := test + test2;
	//Writeln('Hello World');
	test := 5*6;
	test := 5 * 6 + 5;

	test := 5 + 6*5;

	test := -5 + -6;

	test := 6 - test;
	test := 6*test;

	test := test * 77;

	test := test * test;

	test := test/test;
	
	//float := 3.59;
	
	//float := 10e-2;
	
	//float := -10e3;

	//float := 3;			//No type promotion for now
	test := 5 * (6 + 5);

	character := '''';
	character := 'T';
	character := #43;
	character := ^G;

	bool := NOT bool;
	bool := NOT TRUE;

	test := 3;
	//test2 := test;
	//test := NOT test;

	//test := true;

	//test := test * float;
	//unknown := test;
	//test := unknown;

	//test := test + float;

	test := 1 + 2 * test;

	//test := 5;
	//test := test MOD 2;

	write(test);
	write(123);
	write(character);
	write('C');
	write(test2);
	write(99);
	
	test := test / 5 * 6;
	if test = test2 then test2 := 6+5 else test2 := 15;
	if test = 1 then test2 := 6*5;
	
   B:=4;
   for A := 1 to 10 do
      begin
	 write(A);
	 B := B + 1;
      end;
   write(B);
   
   bool := A > B;*}
   A:=1;
   B:=4;
   while (A < B) do
      begin
	 write(A);
	 A := A + 1;
      end;
   write(B);
END.
