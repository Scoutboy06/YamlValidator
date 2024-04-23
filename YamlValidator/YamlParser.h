#pragma once
#include <string>
#include <variant>
#include <vector>
#include <fstream>
#include <map>
#include <regex>

#include "Types.h"

const std::string specialChars("{}[],&*#?|-<>=!%@\\:");

enum ParserError {
    FileOpenError,
    InvalidIndentationError,
    ItemWithoutDataError,
    UnexpectedEndOfFileError,
    InvalidAnchorError,
    InvalidAliasError,
    InvalidTagError,
    InvalidScalarError,
    InvalidSequenceError,
    InvalidMappingError,
    DuplicateKeyError,
    InvalidDocumentStartError,
    InvalidDocumentEndError,
    UnexpectedCharacterError,
    InvalidCommentError,
    InvalidLiteralBlockError,
    InvalidFoldedBlockError,
    InvalidTimestampError,
    InvalidBooleanError,
    InvalidNullValueError,
    InvalidBinaryError,
    InvalidOmapError,
    InvalidPairsError,
    InvalidSetError,
    InvalidImplicitTypeError,
    InvalidExplicitTypeError,
    InvalidReservedIndicatorError,
    InvalidNodePropertiesError,
    InvalidContentError,
    InvalidDocumentIndicatorError,
    InvalidDirectiveError,
    InvalidReservedDirectiveError,
    InvalidYamlVersionError,
    InvalidYamlDirectiveError,
    InvalidTagDirectiveError,
    InvalidTagHandleError,
    InvalidTagPrefixError,
    InvalidDirectiveLineError,
    InvalidBomError,
    InvalidEncodingError,
    InvalidInputError,
    ParserInternalError,
};

class ParserResult {
private:
	std::variant<Yaml, ParserError> result;

public:
	ParserResult(Yaml ok) : result(ok) {}
	ParserResult(ParserError error) : result(error) {}

	bool IsOk() const { return std::holds_alternative<Yaml>(result); }

	bool IsError() const { return std::holds_alternative<ParserError>(result); }

	Yaml GetResult() const { return std::get<Yaml>(result); }

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
    long lineIndex = 0;
    long colIndex = 0;
	bool isEOF = false;
	bool isPeekEOF = false;

	void Advance();
    void SkipSpaces();
	void SkipWhitespace();
    void Expect(char c, ParserError error);
    void ExpectEither(const std::string& chars, ParserError error);

    YamlValue ParseValue();
    std::string ParseObjectKey();
    static std::optional<bool> IsBoolean(std::string& value);
    static bool IsNumber(std::string& value);
    static bool IsNull(std::string& value);
    bool IsTimestamp(std::string& value);

	Object ParseYamlObject();
    Array ParseYamlArray();

    /*
     * Reads the input stream and parses the contents
     * as a JSON-like object.
     * 
     * @note `currChar` MUST be `{` when calling this method.
     * Otherwise, an error will be thrown.
     */
	Object ParseJsonObject();

    /*
     * Reads the input stream and parses the contents
     * as a JSON array.
     * 
     * @note `currChar` MUST be `[` when calling this method.
     * Otherwise, an error will be thrown.
     */
	Array ParseJsonArray();
	
public:
	YamlParser(std::ifstream& stream) : stream(stream) {
		stream.get(currChar);
		stream.get(peekChar);
	}

	ParserResult Parse();
};

ParserResult ParseYaml(const std::string& filePath);