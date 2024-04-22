#pragma once
#include <string>
#include <variant>
#include <vector>
#include <fstream>
#include <map>

#include "Types.h"

using namespace ParserTypes;

enum ParserError {
	InvalidIndentation,
	ItemWithoutData,
	UnexpectedEndOfFile,
};

class ParserResult {
private:
	std::variant<Yaml, ParserError> result;

public:
	ParserResult(Yaml ok) : result(ok) {}
	ParserResult(ParserError error) : result(error) {}

	bool IsOk() const { return std::holds_alternative<Yaml>(result); }

	bool IsError() const { return std::holds_alternative<ParserError>(result); }

	Yaml GetResult() const { return std::get<::Yaml>(result); }

	ParserError GetError() const { return std::get<ParserError>(result); }

	std::optional<Yaml> GetIfOk() const {
		if (std::holds_alternative<Yaml>(result)) return std::get<Yaml>(result);
		return std::nullopt;
	}

	std::optional<ParserError> GetIfError() const {
		if (std::holds_alternative<ParserError>(result)) return std::get<ParserError>(result);
		return std::nullopt;
	}
};


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