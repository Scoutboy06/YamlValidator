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
	Object obj;
	obj.Set({ "key", String("value") });
	return obj;
}

Object YamlParser::ParseJsonObject() {
	Object obj;
	obj.Set({ "key", String("value") });
	return obj;
}

Array YamlParser::ParseYamlArray() {
	Array arr;
	arr.PushBack(String("Array value"));
	return arr;
}

Array YamlParser::ParseJsonArray() {
	Array arr;
	arr.PushBack(String("Array value"));
	return arr;
}

ParserResult YamlParser::Parse() {
	return ParserResult(Ok());
}

ParserResult ParseYaml(std::string& filePath)
{
	std::ifstream stream(filePath);
	YamlParser parser(stream);
	return parser.Parse();
}
