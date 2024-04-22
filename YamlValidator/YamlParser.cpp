#include "YamlParser.h"

void YamlParser::Advance() {}

void YamlParser::SkipWhitespace() {}

void YamlParser::Expect(char c) {}

void YamlParser::ExpectEither(std::initializer_list<char> list) {}

ParserTypes::String YamlParser::ParseString() {
	return ParserTypes::String("test");
}

ParserTypes::Number YamlParser::ParseNumber() {
	return ParserTypes::Number("123");
}

ParserTypes::Boolean YamlParser::ParseBoolean() {
	return ParserTypes::Boolean(true);
}

ParserTypes::Null YamlParser::ParseNull() {
	return ParserTypes::Null();
}

ParserTypes::Object YamlParser::ParseYamlObject() {
	return ParserTypes::Object();
}

ParserTypes::Object YamlParser::ParseJsonObject() {
	return ParserTypes::Object();
}

ParserTypes::Array YamlParser::ParseYamlArray() {
	ParserTypes::Array arr;
	return arr;
}

ParserTypes::Array YamlParser::ParseJsonArray() {
	ParserTypes::Array arr;
	return arr;
}

ParserResult YamlParser::Parse() {
	std::shared_ptr<ParserTypes::Array> yaml = std::make_shared<ParserTypes::Array>(ParseYamlArray());
	return ParserResult(yaml);
}

ParserResult ParseYaml(std::string& filePath) {
	std::ifstream stream(filePath);
	YamlParser parser(stream);
	return parser.Parse();
}
