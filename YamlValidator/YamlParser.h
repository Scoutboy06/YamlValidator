#pragma once
#include <string>
#include <variant>
#include <vector>
#include <fstream>
#include <map>

#include "Types.h"

struct Ok {};

struct Error {
	std::string& message;
};

using ParserResult = std::variant<Ok, Error>;


class YamlParser {
private:
	std::ifstream& stream;
	char currChar;
	char peekChar;
	long rowNumber = 0;
	long colNumber = 0;
	bool isEOF = false;
	bool isPeekEOF = false;

	void Advance();
	void SkipWhitespace();
	void Expect(char c);
	void ExpectEither(std::initializer_list<char> list);

	String ParseString();
	Number ParseNumber();
	Boolean ParseBoolean();
	Null ParseNull();

	Object ParseYamlObject();
	Object ParseJsonObject();
	Array ParseYamlArray();
	Array ParseJsonArray();
	
public:
	YamlParser(std::ifstream& stream) : stream(stream) {
		stream.get(currChar);
		stream.get(peekChar);
	}

	ParserResult Parse();
};

ParserResult ParseYaml(std::string& filePath);


