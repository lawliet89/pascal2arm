PROGRAM HelloWorld(output);

PROCEDURE Dummy;
	BEGIN
		writeln('Hello dummy!');
	END; {dummy}

BEGIN
	writeln('Hello World');
	dummy;
END.