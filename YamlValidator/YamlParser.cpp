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

YamlValue YamlParser::ParseValue() {
    SkipWhitespace();

    // JSON Array or Object
    if (currChar == '[') return std::make_shared<Array>(ParseJsonArray());
    if (currChar == '{') return std::make_shared<Object>(ParseJsonObject());
    // TODO: handle YAML array and object

    // Must be a Scalar
    // https://symfony.com/doc/current/reference/formats/yaml.html#scalars

    std::string value;
    bool isSingleQuoted = currChar == '\'';
    bool isDoubleQuoted = currChar == '"';

    if (isSingleQuoted || isDoubleQuoted) Advance();

    // TODO: Handle multi-line strings
    //bool isMultiLine = currChar == '>' || currChar == '|';

    // Extract the value
    while (!(currChar == ',' && !isSingleQuoted && !isDoubleQuoted)) {
        // Handle " character
        // " characters don't need to be escaped in a single-quote string
        if (currChar == '"' && !isSingleQuoted) {
            
            // " characters are not valid inside an un-quoted string
            if (!isDoubleQuoted) {
                throw ParserError::InvalidScalarError;
            }

            // End of quotation
            return String(value); // Quotations always return strings
        }

        // Handle ' character
        // ' characters don't need to be escaped in a double-quote string
        else if (currChar == '\'' && !isDoubleQuoted) {
            
            // ' characters are not valid inside an un-quoted string
            if(!isSingleQuoted) {
                throw ParserError::InvalidScalarError;
            }

            // ' characters are escaped by ' characters in a single-quote string
            // eg. 'this ''string'' has '' characters inside,
            // Which gives out "this 'string' has ' characters inside"
            if(peekChar == '\'') {
                value.push_back('\'');
                Advance();
                Advance();
                continue;
            }

            // End of quotation
            return String(value); // Quotations always return strings
        }

        // Handle invalid characters
        else if (!isSingleQuoted && !isDoubleQuoted && specialChars.find(currChar) != std::string::npos) {
            throw ParserError::UnexpectedCharacterError;
        }

        value.push_back(currChar);
        Advance();
    }

    // End of un-quoted value
    if (auto opt = IsBoolean(value); opt.has_value()) return Boolean(opt.value());
    else if (IsNumber(value)) return Number(value);
    else if (IsNull(value)) return Null();
    //else if(auto opt = IsTimestamp(value); opt.has_value()) return Timestamp(opt.value());
    return String(value);
}

std::string YamlParser::ParseObjectKey() {
    std::string key;

    bool isSingleQuoted = currChar == '\'';
    bool isDoubleQuoted = currChar == '"';

    while (!(currChar != ':' && !isSingleQuoted && !isDoubleQuoted)) {

        // Check for invalid quotation marks
        if ((currChar == '\'' || currChar == '"') && !isDoubleQuoted && !isSingleQuoted) {
            throw ParserError::UnexpectedCharacterError;
        }

        // Handle single-quotes
        else if (currChar == '\'' && isSingleQuoted) {
            // Single quotation marks are escaped by single
            // quotation marks in a YAML string
            if (peekChar == '\'') {
                key.push_back('\'');
                Advance();
                Advance();
                continue;
            }

            // End of key
            else {
                break;
            }
        }

        // Backslash-escaped characters not handled by std::ifstream
        else if (currChar == '\\' && isDoubleQuoted) {
            // Escaped " character in double quoted string
            if (peekChar == '"' && isDoubleQuoted) {
                key.push_back('"');
                Advance();
                Advance();
                continue;
            }

            // End of key
            else {
                break;
            }
        }

        // Search for un-quoted special characters
        else if (!isSingleQuoted && !isDoubleQuoted && specialChars.find(currChar) != std::string::npos) {
            throw ParserError::UnexpectedCharacterError;
        }

        key.push_back(currChar);
        Advance();
    }

    return key;
}

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
    Object obj;

    Expect('{', ParserError::ParserInternalError);
    SkipWhitespace();

    while (currChar != '}') {
        std::string key = ParseObjectKey();

        SkipWhitespace();
        Expect(':', ParserError::ParserInternalError);

        YamlValue value = ParseValue();

        obj.Set(key, value);

        SkipWhitespace();
        if (currChar == ',') {
            Advance();
            SkipWhitespace();
        }
    }

    return obj;
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
