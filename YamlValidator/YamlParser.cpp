#include "YamlParser.h"

void YamlParser::Advance() {}

void YamlParser::SkipWhitespace() {}

void YamlParser::Expect(char c) {}

void YamlParser::ExpectEither(std::initializer_list<char> list) {}

_String YamlParser::ParseString() {
	return _String("test");
}

_Number YamlParser::ParseNumber() {
	return _Number("123");
}

_Boolean YamlParser::ParseBoolean() {
	return _Boolean(true);
}

_Null YamlParser::ParseNull() {
	return _Null();
}

_Object YamlParser::ParseYamlObject() {
	std::map<_String, YamlValue> obj;
	return _Object(obj);
}

_Object YamlParser::ParseJsonObject() {
	std::map<_String, YamlValue> obj;
	obj.insert(std::pair("key", _String("value")));
	return _Object(obj);
}

_Array YamlParser::ParseYamlArray() {
	std::vector<YamlValue> values;
	values.push_back(_String("test"));
	return _Array(values);
}

_Array YamlParser::ParseJsonArray() {
	std::vector<YamlValue> values;
	values.push_back(_String("test"));
	return _Array(values);
}

ParserResult YamlParser::Parse() {
	return ParserResult(Ok());
}
