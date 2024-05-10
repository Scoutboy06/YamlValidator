#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <format>

#include "Types.h"
#include "YamlParser.h"

class Schema {
public:
    /**
     * @enum Type
     * @brief Enum representing the types of data that can be validated.
     */
    enum Type {
        String,
        Number,
        Boolean,
        Null,
        Timestamp
    };

    struct Either;

private:

    struct ObjectImplementation; /// Forward declaration of ObjectImplementation

    struct ArrayImplementation;

public:


    using SchemaValue = std::variant<Type, Either, std::shared_ptr<ArrayImplementation>, std::shared_ptr<ObjectImplementation>>;
    /// SchemaValue needs a shared pointer for ObjectImplementation because it is forward declared
    /// and std::variant typically needs to know the objects size at declaration.


    /**
     * @struct Either
     * @brief Struct representing the Either schema type which can store multiple enumeration-constants of the Type enum.
     *        Is used to allow for a yaml value to be of different types.
     */
    struct Either {
        std::vector<SchemaValue> values; /// The different value-types to allow for. 

        template<typename... Args>
        Either(Args... args) : values({ args... }) { };
    };

private:

    /**
     * @struct ArrayImplementation
     * @brief Struct representing an Array which accepts either a Type enum or Either struct
     *        to represent the allowed types of the array.
     * @note Does not allow for arrays containing arrays or objects.
     */
    struct ArrayImplementation {
    public:
        SchemaValue type; /// The yaml data types that are allowed in the array.

        /**
         * @brief Constructor for ArrayImplementation.
         * @param type The yaml data types that are allowed in the array.
         */
        ArrayImplementation(SchemaValue type) : type(type) {};
    };

    /**
     * @struct ObjectImplementation
     * @brief Struct representing an Object with an std::string as key and SchemaValue as value.
     */
    struct ObjectImplementation {
    public:
        std::unordered_map<std::string,SchemaValue> values;

        /**
         * @brief Constructor for ObjectImplementation.
         * @param values An unordered map with an std::string as key and SchemaValue as value.
         */
        ObjectImplementation(std::unordered_map<std::string, SchemaValue> values) : values(values) {};
        
        /**
         * @brief Gets the SchemaValue stored for the key if the key exists.
         * @param key The key to get the value with.
         * @return An std::optional containing a SchemaValue if a value
         *         associated with the key exists, std::nullopt otherwise.
         */
        std::optional<SchemaValue> Get(const std::string& key) const {
            auto it = values.find(key);
            if (it != values.end()) {
                return it->second;
            }
            return std::nullopt;
        }

        /**
         * @brief Checks whether or not the key is valid.
         * @param key The key to check whether or not it exists in the object.
         * @return True if the object contains the key, false otherwise.
         */
        bool ContainsKey(const std::string& key) const {
            return values.find(key) != values.end();
        }

        /**
         * @brief Gets all keys in the object.
         * @return Vector containing the SchemaValue values.
         */
        std::vector<std::string> ExtractKeys() const {
            std::vector<std::string> keys;
            keys.reserve(values.size());
            for (const auto& pair : values) {
                keys.push_back(pair.first);
            }
            return keys;
        }
    };

public:

    /**
     * @brief Creates a shared pointer of an ArrayImplementation constructed based on parameters.
     * @param type Variant containing the allowed types in the vector.
     * @return Shared pointer of an ArrayImplementation.
     */
    static std::shared_ptr<ArrayImplementation> CreateArray(SchemaValue type) {
        return std::make_shared<ArrayImplementation>(ArrayImplementation(type));
    };

    /**
     * @brief Creates a shared pointer of an ObjectImplementation constructed based on parameters.
     * @param values Unordered map with the structure: { std::string, SchemaValue }.
     * @return Shared pointer of an ObjectImplementation.
     */
    static std::shared_ptr<ObjectImplementation> CreateObject(std::unordered_map<std::string, SchemaValue> values) {
        return std::make_shared<ObjectImplementation>(ObjectImplementation(values));
    };

private:
    std::variant<std::shared_ptr<ObjectImplementation>, std::shared_ptr<ArrayImplementation>> schema; /// The root for the schema which is used to validate.

    /**
     * @brief Gives a string containing the visual name of the given type.
     * @param instance Instance of either Type, Either or YamlValue.
     * @return String containing the visual name of the given type.
     */
    static std::string getTypeName(std::variant<SchemaValue, parser_types::YamlValue> instance);

    /**
     * @brief Checks whether or not the provided YamlValue instance has 
     *		  an equivalent Type based on the type parameter.
     * @param type Instance of Type or Either to compare.
     * @param yamlInstance Instance of YamlValue to compare.
     * @note If the type parameter is of instance Either then the return will be true if the 
     *		 yamlInstance is equivalent to any of the contained types.
     * @return True if the YamlValue instance has an equivalent Type.
     */
    static bool compareTypeToParserType(SchemaValue type, parser_types::YamlValue yamlInstance);
public:

    /**
     * @enum ErrorType
     * @brief Enum class representing the types of errors that can occur during validation.
     */
    enum ErrorType {
        TypeMismatch,
        UnexpectedValue,
        UnknownError
    };

    /**
     * @struct SchemaError
     * @brief Struct representing a schema error.
     */
    struct SchemaError {
        /**
         * @struct ArrayError
         * @brief Struct representing an array error.
         */
        struct ArrayError {
            std::shared_ptr<parser_types::Array> errorRoot; /// The parser Array root of the error.
            std::optional<int> index; /// The index of the parser Array where the error occured.

            /**
             * @brief Constructor for ArrayError.
             * @param errorRoot The parser Array root of the error.
             * @param key The index of the parser Array where the error occured.
             */
            ArrayError(std::shared_ptr<parser_types::Array> errorRoot, std::optional<int> index) : errorRoot(errorRoot), index(index) {};
        };

        struct ObjectError {
            std::shared_ptr<parser_types::Object> errorRoot; /// The parser Object root of the error.
            std::optional<std::string> key; /// The key of the parser Object where the error occured.

            /**
             * @brief Constructor for ObjectError.
             * @param errorRoot The parser Object root of the error.
             * @param key The key of the parser Object where the error occured.
             */
            ObjectError(std::shared_ptr<parser_types::Object> errorRoot, std::optional<std::string> key) : errorRoot(errorRoot), key(key) {};
        };

        std::optional<std::variant<ArrayError, ObjectError>> information; /// Specific information about the error based on if the error occurs in an parser Array or parser Object.
        ErrorType errorType; /// The type of error.
        std::string message; /// String containing stringified message based on information and errorType.

        /**
         * @brief Constructor for SchemaError.
         * @param information Specific information about the error based on if the error occurs 
         *					  in an parser Array or parser Object.
         * @param errorType The type of error.
         * @param message String containing message based on information and errorType.
         */
        SchemaError(std::optional<std::variant<ArrayError, ObjectError>> information, ErrorType errorType, std::string message) : information(information), errorType(errorType), message(message) {};
    };

    /**
     * @struct ValidationResult
     * @brief Struct representing a validation result.
     */
    struct ValidationResult {

        /**
         * @struct ValidationError
         * @brief Struct representing a validation error.
         */
        struct ValidationError {

            std::variant<ParserError, SchemaError> error; /// ParserError or SchemaError with appropriate information.

            /**
             * @brief Constructor for ValidationError.
             * @param error ParserError or SchemaError with appropriate information.
             */
            ValidationError(std::variant<ParserError, SchemaError> error) : error(error) {};
        };

        /**
         * @struct ValidationSuccess
         * @brief Struct representing a validation success.
         */
        struct ValidationSuccess {

        };

        std::variant<ValidationError, ValidationSuccess> result; /// ValidationError or ValidationSuccess with appropriate information

        /**
         * @brief Constructor for ValidationResult.
         * @param result ValidationError or ValidationSuccess with appropriate information.
         */
        ValidationResult(std::variant<ValidationError, ValidationSuccess> result) : result(result) { }
    };

private:
    /**
     * @brief Compares provided SchemaValue and YamlValue.
     * @param schemaValue SchemaValue to compare.
     * @param yamlValue YamlValue to compare.
     * @param mismatch ValidationResult error to return when a TypeMismatch occurs. 
     * @return A ValidationResult object containing a ValidationError with appropriate information.
     */
    static Schema::ValidationResult ValidateCompare(SchemaValue schemaValue, YamlValue yamlValue, ValidationResult mismatch);
public:

    /**
     * @brief Creates a ValidationResult with provided ErrorType and information.
     * @param errorInformation Specific information about the error based on if the error occurs 
     *					       in an parser Array or parser Object.
     * @param errorType The type of error.
     * @param message String containing message based on information and errorType.
     * @return A ValidationResult object containing a ValidationError with appropriate information.
     */
    static ValidationResult GetValidationError(std::optional<std::variant<SchemaError::ArrayError, SchemaError::ObjectError>> errorInformation, ErrorType errorType, std::string message = "");

    /**
     * @brief Creates a ValidationResult with the UnexpectedValue errorType and information and a message based on parameters.
     * @param errorInformation Specific information about the error based on if the error occurs
     *					       in an parser Array or parser Object.
     * @param expected The expected type of the value.
     * @param got The actual type of the value.
     * @return A ValidationResult object containing a ValidationError with appropriate information.
     */
    static ValidationResult GetValidationErrorMismatch(std::optional<std::variant<SchemaError::ArrayError, SchemaError::ObjectError>> errorInformation, SchemaValue expected, parser_types::YamlValue got);

    /**
     * @brief Creates a ValidationResult with the UnexpectedValue errorType and information and a message based on parameters.
     * @param errorInformation Specific information about the error based on if the error occurs
     *					       in an parser Array or parser Object.
     * @param unexpected The unexpected YamlValue.
     * @return A ValidationResult object containing a ValidationError with appropriate information.
     */
    static ValidationResult GetValidationErrorUnexpected(std::optional<std::variant<SchemaError::ArrayError, SchemaError::ObjectError>> errorInformation, parser_types::YamlValue unexpected);

    /**
     * @brief Constructor for Schema.
     * @param schema The root for the schema which is used to validate.
     */
    Schema(std::variant<std::shared_ptr<ObjectImplementation>, std::shared_ptr<ArrayImplementation>> schema) : schema(schema) {
    };

    /**
     * @brief Validates provided Yaml object based on provided schema.
     * @param yaml The parsed Yaml object.
     * @param schema The root for the schema which is used to validate.
     * @return A ValidationResult object containing either a ValidationError or a ValidationSuccess object.
     */
    static ValidationResult Validate(parser_types::Yaml yaml, std::variant<std::shared_ptr<ObjectImplementation>, std::shared_ptr<ArrayImplementation>> schema);

    /**
     * @brief Validates provided Yaml file.
     * @return A ValidationResult object containing either a ValidationError or a ValidationSuccess object.
     */
    ValidationResult ValidateFromFile(const std::string& path);
};

