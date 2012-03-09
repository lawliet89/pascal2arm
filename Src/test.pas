PROGRAM HelloWorld;

VAR
	test, test2: integer;
	float: real;
	character: char;
	bool: boolean;

PROCEDURE testing;
	VAR
		test: boolean;
	BEGIN
		//test := 123;
	END;
	
BEGIN
	test := 5;		//Simple
	test2 := 6;
	test := 1+2;
	test := test + test2;
	//Writeln('Hello World');
	//test := 5 * 6 + 5;

	//test := 5 + 6*5;

	//test := -5 + -6;
	
	//float := 3.59;
	
	//float := 10e-2;
	
	//float := -10e3;

	//float := 3;			//No type promotion for now
	//test := 5 * (6 + 5);

	character := '''';
	character := 'T';
	character := #43;
	character := ^G;

	bool := NOT bool;
	test2 := test;
	//test := NOT test;

	//test := true;

	//test := test * float;
	//unknown := test;
	//test := unknown;

	//test := test + float;

END.
