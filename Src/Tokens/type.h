#ifndef TokenTypeH
#define TokenTypeH
#include <string>
#include "../token.h"
#include "../define.h"


/** Types **/
/*
	Primary Type (mutually exclusive):
	- Integer
	- Real
	- Boolean
	- Record
	- Enum
	- Char
	- String
	- File
	- Set
	
	Secondary Type (not mutually exclusive):
	- Subrange
	- Array
	- Pointer
 */
//TODO Type attributes for more complicated types
class Token_Type: public Token{
public:
	//Primary Type
	enum P_Type{
		Integer,
		Real,
		Boolean,
		Record,
		Enum,
		Char,
		String,
		File,
		Set,
		Void		//Internal use
	};
	
	enum S_Type{
		Subrange=1u<<0,	//0b001
		Array=1u<<1,	//0b010
		Pointer=1u<<2	//0b100
	};
	
	//OCCF
	Token_Type(std::string id, P_Type pri, int sec=0, int size=4);
	Token_Type(const Token_Type &obj);
	
	~Token_Type(){}
	Token_Type operator=(const Token_Type &obj);
	
	//Getters
	P_Type GetPrimary() const{ return Primary; }
	unsigned GetSecondary() const { return Secondary; }
	bool IsSubrange() const { return Secondary & Subrange; }
	bool IsArray() const { return Secondary & Array; }
	bool IsPointer() const { return Secondary & Pointer; }
	//ID
	std::string GetID() const { return GetStrValue(); }
	//Size
	void SetSize(int size) { this -> size = size; }
	int GetSize() const { return size; }
	
	//Setters
	void SetArray(bool val=true);
	void SetPointer(bool val=true);
	void SetSubrange(bool val=true);
	void CleanSecondary();
	
	//Default value for Type
	virtual std::string AsmValue() { return AsmDefaultValue(); }
	virtual std::string AsmDefaultValue();

	std::string TypeToString();
	
	//Comparing operators
	bool operator==(const Token_Type &obj) const;
	bool operator!=(const Token_Type &obj) const;
	bool operator==(const Token &obj) const;		//Overridden
	bool operator!=(const Token &obj) const;		//Overridden
protected:
	P_Type Primary;
	unsigned Secondary;
	int size;			//No of bytes.
};

#endif