PROGRAM HelloWorld;

VAR
	test: integer;
	float: real;
	character: char;

PROCEDURE testing;
	VAR
		test: boolean;
	begin
		//test := 123;
	END;
	
BEGIN
	Writeln('Hello World');
	test := 5 * 6 + 5;

	test := 5 + 6*5;

	test := -5 + -6;
	
	float := 3.59;
	
	float := 10e-2;
	
	float := -10e3;

	character := '''';
	character := 'T';
	character := #43;
	character := ^G;

	//test := true;

	//test := test * float;
	//unknown := test;
	//test := unknown;

	//test := test + float;

END.
