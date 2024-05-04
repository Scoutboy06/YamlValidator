#pragma once
#include <string>
#include <variant>
#include <vector>
#include <fstream>
#include <map>
#include <regex>
#include <iostream>
#include <stack>

#include "Types.h"

using namespace parser_types;

#define INDENT_SIZE 2

const std::string invalidKeyChars(R"({}[]&*#?|<>=!%@\)");
const std::string invalidValueChars(R"(:{}[]&*#?|<>=!%@\"')");

/**
 * @enum ErrorType
 * @brief Enum class representing the types of errors that can occur during parsing.
 */
enum ErrorType {
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

/**
 * @struct ParserError
 * @brief Struct representing a parser error.
 */
struct ParserError {
    ErrorType error;    /// The type of error.
    long line;          /// The line number where the error occured.
    long column;        /// The column number where the error occured.

    /**
     * @brief Constructor for ParserError.
     * @param error The type of error.
     * @param line The line where the error occured
     * @param column The column where the error occured
     */
    ParserError(ErrorType error, long line, long column) : error(error), line(line), column(column) {}
};

/**
 * @class ParserResult
 * @brief Class that contains a variant with either the parsed Yaml data,
 *        or a ParserError
 */
class ParserResult {
private:
    std::variant<Yaml, ParserError> result; /// The result of the parse operation.

public:
    /**
     * @brief Constructor for ParserResult when ther parse operation is successful.
     * @param ok The parsed Yaml object.
     */
    ParserResult(Yaml ok) : result(ok) {}

    /**
     * @brief Constructor for ParserResult when the parse operation fails.
     * @param error The type of error.
     * @param line The line number where the error occured.
     * @param column The column number where the error occured.
     */
    ParserResult(ErrorType error, long line, long column) : result(ParserError(error, line, column)) {}

    /**
     * @brief Checks if the parse operation was successful.
     * @return True if the parse operation was successful, false otherwise.
     */
    bool IsOk() const { return std::holds_alternative<Yaml>(result); }

    /**
     * @brief Checks if the parse operation failed.
     * @return True if the parse operation failed, false otherwise.
     */
    bool IsError() const { return std::holds_alternative<ParserError>(result); }

    /**
     * @brief Get the result of the parse operation.
     * @return The parsed Yaml object.
     * @throws std::bad_variant_access if the result is not a Yaml object.
     */
    Yaml GetResult() const { return std::get<Yaml>(result); }

    /**
     * @breif Get the error of the parse object.
     * @return The ParserError object.
     * @throws std::bad_variant_access if the result is not a Yaml object.
     */
    ParserError GetError() const { return std::get<ParserError>(result); }

    /**
     * @brief Get the result of the parse operation if it was successfull.
     * @return An optional containing the parsed Yaml object if the parse
     *         operation was successful, std::nullopt otherwise.
     */
    std::optional<Yaml> GetIfOk() const {
        if (std::holds_alternative<Yaml>(result))
            return std::get<Yaml>(result);
        return std::nullopt;
    }

    /**
     * @brief Get the error of the parse operation if it failed.
     * @return An optional containing the ParserError object if the
     *         parse operation failed, std::nullopt otherwise.
     */
    std::optional<ParserError> GetIfError() const {
        if (std::holds_alternative<ParserError>(result))
            return std::get<ParserError>(result);
        return std::nullopt;
    }
};

/**
 * @class YamlParser
 * @brief Class responsible for parsing Yaml data.
 */
class YamlParser {
private:
    std::ifstream& stream;                  /// A reference to the input file stream..

    char currChar;                          /// The current character being processed.
    char peekChar;                          /// The next character being processed.

    uint32_t line = 1;                      /// The current line number.
    uint32_t column = 1;                    /// The current column number.

    bool isEOF = false;                     /// Flag indicating if the end of file has been reached.
    bool isPeekEOF = false;                 /// Flag indicating if the end of file has been reached.

    std::stack<uint32_t> indentStack;       /// Stack for keeping track of indentation levels.

    /**
     * @brief Advances the parser to the next character in the input file stream.
     */
    void Advance();       

    /**
     * @brief Advances the parser to the next character that is not whitespace.
     */
    void SkipWhitespace();

    /**
     * @brief Checks if the current character matches the expected one.
     * @param c The expected character.
     * @param error The error to be thrown if the characters does not match.
     * @throws ParserError if the characters does not match.
     */
    void Expect(char c, ErrorType error);

    /**
     * @brief Consumes and parses the input stream into a YamlValue.
     * @return The parsed value as a YamlValue variant.
     * @throws ParserError if the value is invalid.
     */
    YamlValue ParseValue();

    /**
     * @brief Consumes and parses an object key from the input stream.
     * @return The parsed key as a string.
     * @throws ParserError if the key is invalid.
     */
    std::string ParseObjectKey();

    /**
     * @brief Checks if the input string is a valid Yaml boolean.
     * @return An optional containing the boolean value if the string 
     *         represents a valid Yaml boolean if valid, std::nullopt otherwise.
     */
    static std::optional<bool> IsBoolean(const std::string& value);

    /**
     * @brief Checks if the input string is a valid Yaml number.
     * @return True if the input string is a valid Yaml number, false otherwise.
     */
    static bool IsNumber(const std::string& value);

    /**
     * @brief Checks if the input string is a valid Yaml null value.
     * @return True if the input string is a valid Yaml null value, false otherwise.
     */
    static bool IsNull(const std::string& value);

    /**
     * @brief Checks if the input string is valid ISO8601 timestamp.
     * @return True if the input string is a valid ISO8601 timestamp, false otherwise.
     */
    static bool IsTimestamp(const std::string& value);

    /**
     * @brief Consumes and parses the input stream as a Yaml Object.
     * @param firstKey The first key of the object. This is necessary due
     *                 to how LL(1) parsers work.
     * @return Object containing a mapping of key-value pairs.
     * @throw ParserError if the input stream represents an invalid Yaml object.
     */
    Object ParseYamlObject(const std::string& firstKey);

    /**
     * @brief Consumes and parses the input stream as a Yaml Array.
     * @return Array containing a list of YamlValue items.
     * @throw ParserError if the input stream represents an invalid Yaml array.
     */
    Array ParseYamlArray();

    /*
     * @brief Consumes and parses the input stream as a Json-like object.
     * @note `currChar` MUST be `{` when calling this method. Otherwise, an error will be thrown.
     * @throws ParserError if currChar != '{'
     */
    Object ParseJsonObject();

    /*
     * @brief Consumes and parses the input stream as a Json-like array.
     * @note `currChar` MUST be `[` when calling this method, else an error will be thrown.
     * @throws ParserError if currChar != '['
     */
    Array ParseJsonArray();
    
public:
    /**
     * @brief Constructor for YamlParser
     * @param stream Reference to the input file stream.
     */
    YamlParser(std::ifstream& stream) : stream(stream) {
        stream.get(currChar);
        stream.get(peekChar);
    }

    /**
     * @brief Parses the input stream into a Yaml object.
     * @return A ParserResult object containing either the parsed
     *         Yaml object or a ParseError object.
     */
    ParserResult Parse();
};

/**
 * @brief Parses a Yaml file.
 * @param filePath The file path to the input file.
 * @return A ParserResult object containing either the parses
 *         Yaml object or a ParseError object.
 */
ParserResult ParseYaml(const std::string& filePath);