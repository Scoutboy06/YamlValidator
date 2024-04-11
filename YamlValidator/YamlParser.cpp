#include "YamlParser.h"

void YamlParser::Advance() {}

void YamlParser::SkipWhitespace() {}

void YamlParser::Expect(char c) {}

void YamlParser::ExpectEither(std::initializer_list<char> list) {}

String YamlParser::ParseString() {
	return String("test");
}

Number YamlParser::ParseNumber() {
	return Number("123");
}

Boolean YamlParser::ParseBoolean() {
	return Boolean(true);
}

Null YamlParser::ParseNull() {
	return Null();
}

Object YamlParser::ParseYamlObject() {
	return Object();
}

Object YamlParser::ParseJsonObject() {
	return Object();
}

Array YamlParser::ParseYamlArray() {
	Array arr;
	Number n("1");
	arr.PushBack(Number("1"));
	arr.PushBack(Number("2"));
	arr.PushBack(Number("3"));
	return arr;
}

Array YamlParser::ParseJsonArray() {
	Array arr;
	arr.PushBack(1);
	arr.PushBack(2);
	arr.PushBack(3);
	return arr;
}

ParserResult YamlParser::Parse() {
	return ParserResult(Ok(ParseYamlArray()));
}
