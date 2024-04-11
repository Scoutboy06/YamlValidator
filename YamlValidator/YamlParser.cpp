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
	return arr;
}

Array YamlParser::ParseJsonArray() {
	Array arr;
	return arr;
}

ParserResult YamlParser::Parse() {
	std::shared_ptr<Array> yaml = std::make_shared<Array>(ParseYamlArray());
	return ParserResult(yaml);
}

ParserResult ParseYaml(std::string& filePath) {
	std::ifstream stream(filePath);
	YamlParser parser(stream);
	return parser.Parse();
}
