#include "YamlParser.h"

void YamlParser::Advance() {
    colIndex++;

    // Previous character
    if (currChar == '\n') {
        lineIndex++;
        colIndex = 0;
    }

    currChar = peekChar;
    stream.get(peekChar);

    isEOF = isPeekEOF;
    isPeekEOF = stream.eof();
}

void YamlParser::SkipSpaces() {
    while (currChar == ' ') Advance();
}

void YamlParser::SkipWhitespace() {
    while (isspace(currChar)) Advance();
}

void YamlParser::Expect(char c, ParserError error) {
    if (currChar == c) {
        throw error;
    }
    Advance();
}

void YamlParser::ExpectEither(const std::string& chars, ParserError error) {
    for (char ch : chars) {
        if (ch == currChar) {
            Advance();
            return;
        }
    }

    throw error;
}

void YamlParser::SkipWhitespace() {}

std::optional<bool> YamlParser::IsBoolean(std::string& v) {
    if (v == "true" || v == "True" || v == "TRUE") return true;
    else if (v == "false" || v == "False" || v == "FALSE") return false;
    return std::nullopt;
}

bool YamlParser::IsNumber(std::string& v) {
    if (v == ".nan"  || v == ".NaN"  || v == ".NAN"
     || v == ".inf"  || v == ".Inf"  || v == ".INF"
     || v == "-.inf" || v == "-.Inf" || v == "-.INF") return true;

    // Hexadecimal numbers
    if (v[0] == '0' && v[1] == 'x' && v.size() > 2) {
        for (int i = 2; i < v.size(); i++) {
            if (!(v[i] >= '0' && v[i] <= '9') &&
                !(v[i] >= 'A' && v[i] <= 'F') &&
                !(v[i] >= 'a' && v[i] <= 'f')) return false;
        }
        return true;
    }

    // Octadecimal numbers
    else if (v[0] == '0' && v[1] == 'o' && v.size() > 2) {
        for (int i = 2; i < v.size(); i++) {
            if (v[i] < '0' || v[i] > '7') return false;
        }
        return true;
}

    return std::regex_match(v, std::regex(R"([-+]?(\.[0-9]+|[0-9]+(\.[0-9]*)?)([eE][-+]?[0-9]+)?)"));
	return Number("123");
}

bool YamlParser::IsNull(std::string& v) {
    return v.empty() || v == "~" || v == "null" || v == "Null" || v == "NULL";
}

bool YamlParser::IsTimestamp(std::string& v) {
    // Regex taken from https://www.myintervals.com/blog/2009/05/20/iso-8601-date-validation-that-doesnt-suck/
    return std::regex_match(v, std::regex(R"(^([\+-]?\d{4}(?!\d{2}\b))((-?)((0[1-9]|1[0-2])(\3([12]\d|0[1-9]|3[01]))?|W([0-4]\d|5[0-2])(-?[1-7])?|(00[1-9]|0[1-9]\d|[12]\d{2}|3([0-5]\d|6[1-6])))([T\s]((([01]\d|2[0-3])((:?)[0-5]\d)?|24\:?00)([\.,]\d+(?!:))?)?(\17[0-5]\d([\.,]\d+)?)?([zZ]|([\+-])([01]\d|2[0-3]):?([0-5]\d)?)?)?)?$)"));
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

	Expect('[', ParserError::ParserInternalError);
	SkipWhitespace();

	while (currChar != ']') {
		arr.PushBack(ParseValue());

		if (currChar == ',') Advance();
		
		SkipWhitespace();
	}

	Advance(); // Skip '}'

	return arr;
}

ParserResult YamlParser::Parse() {
    try {
        try {
            std::shared_ptr<Array> yaml = std::make_shared<Array>(ParseJsonArray());
	return ParserResult(yaml);
        } catch (ParserError error) {
            return ParserResult(error);
        }
    }
    
    // Catch any errors that are not of type ParserError
    catch (...) {
        return ParserResult(ParserError::ParserInternalError);
    }
}

ParserResult ParseYaml(const std::string& filePath) {
	std::ifstream stream(filePath);
	YamlParser parser(stream);
	return parser.Parse();
}
