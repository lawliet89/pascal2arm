PROGRAM HelloWorld;

VAR
	test: integer;
	float: real;
	character: char;

PROCEDURE testing;
	VAR
		test: boolean;
	begin
		test := 123;
//PRINT SYMBOL
//PRINT BLOCK
	END;
	
//PRINT SYMBOL
BEGIN
	Writeln('Hello World');
	test := 5 * 6 + 5;

	test := -5 + -6;
	
	float := 3.59;
	
	float := 10e-2;
	
	float := -10e3;

	character := '''';
	character := 'T';
	character := #43;
	character := ^G;
	
	//unknown := test;
	//test := unknown;

//PRINT BLOCK
END.
