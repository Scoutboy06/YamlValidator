#pragma once
#include <string>
#include <variant>
#include <vector>
#include <fstream>
#include <map>

// Data types for internal use
struct _String { std::string value; };
struct _Number { std::string value; };
struct _Boolean { bool value; };
struct _Null;

class _Object;
class _Array;

using YamlValue = std::variant<_String, _Number, _Boolean, _Null, _Object, _Array>;
using Yaml = std::variant<_Object, _Array>;

class _Object { std::map<_String, YamlValue> values; };
class _Array { std::vector<YamlValue> values; };

enum class ParserResult {};

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

	_String ParseString();
	_Number ParseNumber();
	_Boolean ParseBoolean();
	_Null ParseNull();

	_Object ParseYamlObject();
	_Object ParseJsonObject();
	_Array ParseYamlArray();
	_Array ParseJsonArray();
	
public:
	YamlParser(std::ifstream& stream) : stream(stream) {
		stream.get(currChar);
		stream.get(peekChar);
	}

	ParserResult Parse();
};

ParserResult ParseYaml(std::string& filePath) {
	std::ifstream stream(filePath);
	YamlParser parser(stream);
	return parser.Parse();
}


