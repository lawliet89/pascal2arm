/**
 * Utility functions
 * 
 * */

#ifndef UtilityH
#define UtilityH

#include <string>
#include <sstream>

//Utility functions

std::string ReadFile(const char* path); //Read a text file to a string


template<typename T> std::string ToString(T input){
	std::stringstream output;
	output << input;
	
	return output.str();
}

template<typename T> T FromString(const char* input){
	T output;
	std::stringstream stream(input);
	stream >> output;
	
	return output;
}

template<typename T> T FromString(std::string input){
	return FromString<T>(input.c_str());
}

//This function can be potentially unsafe. Make sure you know what you are doing!
//Compiler will check for type casting validity during compile time... if you want something
//more general, replace the cast with a reinterpret_cast
template <typename T> T DereferenceVoidPtr(const void *ptr){
	T *output = (T*) ptr;
	return *output;
}

#endif