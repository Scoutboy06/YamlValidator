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
	enum Type {
		String,
		Number,
		Boolean,
		Null,
		Timestamp
	};


	struct ObjectImplementation; //needs to be declared here so it can be forward declared later.

	struct Either {
		std::vector<Type> values;

		template<typename... Args>
		Either(Args... args) : values({args...}) { };
	};

	struct ArrayImplementation {
	public:
		std::variant<Type, Either> type;
		ArrayImplementation(std::variant<Type, Either> type) : type(type) {};
	};


	using SchemaValue = std::variant<Type, Either, std::shared_ptr<ArrayImplementation>, std::shared_ptr<ObjectImplementation>>;
	// SchemaValue needs a shared pointer for Object because it is forward declared
	// and std::variant typically needs to know the objects size at declaration.

	struct ObjectImplementation {
	public:
		std::unordered_map<std::string,SchemaValue> values;
		ObjectImplementation(std::unordered_map<std::string, SchemaValue> values) : values(values) {};
		std::optional<SchemaValue> Get(const std::string& key) const {
			auto it = values.find(key);
			if (it != values.end()) {
				return it->second;
			}
			return std::nullopt;
		}

		bool ContainsKey(const std::string& key) const {
			return values.find(key) != values.end();
		}

		std::vector<std::string> ExtractKeys() const {
			std::vector<std::string> keys;
			keys.reserve(values.size());
			for (const auto& pair : values) {
				keys.push_back(pair.first);
			}
			return keys;
		}
	};

	static std::shared_ptr<ArrayImplementation> CreateArray(std::variant<Type, Either> values) {
		return std::make_shared<ArrayImplementation>(ArrayImplementation(values));
	};

	static std::shared_ptr<ObjectImplementation> CreateObject(std::unordered_map<std::string, SchemaValue> values) {
		return std::make_shared<ObjectImplementation>(ObjectImplementation(values));
	};


	using SchemaPair = std::pair<std::string, SchemaValue>;

private:
	std::variant<std::shared_ptr<ObjectImplementation>, std::shared_ptr<ArrayImplementation>> schema;
	Schema YamlToSchema(parser_types::Yaml yaml) {};

	static std::string getTypeName(std::variant<Type, Either, parser_types::YamlValue> instance);

	static bool compareTypeToParserType(std::variant<Type, Either> type, parser_types::YamlValue yamlInstance);
public:

	enum ErrorType {
		TypeMismatch,
		UnexpectedValue,
		UnknownError
	};
	struct SchemaError {

		struct ArrayError {
			std::shared_ptr<parser_types::Array> errorRoot;
			std::optional<int> index;

			ArrayError(std::shared_ptr<parser_types::Array> errorRoot, std::optional<int> index) : errorRoot(errorRoot), index(index) {};
		};

		struct ObjectError {
			std::shared_ptr<parser_types::Object> errorRoot;
			std::optional<std::string> key;

			ObjectError(std::shared_ptr<parser_types::Object> errorRoot, std::optional<std::string> key) : errorRoot(errorRoot), key(key) {};
		};

		std::optional<std::variant<ArrayError, ObjectError>> information;
		ErrorType errorType;

		std::optional<std::string> message;

		SchemaError(std::optional<std::variant<ArrayError, ObjectError>> information, ErrorType errorType, std::string message) : information(information), errorType(errorType), message(message) {};
	};
	struct ValidationResult {
		struct ValidationError {

			std::variant<ParserError, SchemaError> error;

			ValidationError(std::variant<ParserError, SchemaError> error) : error(error) {};
		};

		struct ValidationSuccess {

		};

		std::variant<ValidationError, ValidationSuccess> result;

		ValidationResult(std::variant<ValidationError, ValidationSuccess> result) : result(result) { }
	};

	static Schema::ValidationResult GetValidationError(std::optional<std::variant<Schema::SchemaError::ArrayError, Schema::SchemaError::ObjectError>> errorInformation, Schema::ErrorType errorType, std::string message = "");

	static Schema::ValidationResult GetValidationErrorMismatch(std::optional<std::variant<Schema::SchemaError::ArrayError, Schema::SchemaError::ObjectError>> errorInformation, std::variant<Schema::Type, Schema::Either> expected, parser_types::YamlValue got);

	static Schema::ValidationResult GetValidationErrorUnexpected(std::optional<std::variant<SchemaError::ArrayError, SchemaError::ObjectError>> errorInformation, parser_types::YamlValue unexpected);

	// static void FromFile(std::string filePath);

	Schema(std::variant<std::shared_ptr<ObjectImplementation>, std::shared_ptr<ArrayImplementation>> schema) : schema(schema) {
	};

	static ValidationResult Validate(parser_types::Yaml yaml, std::variant<std::shared_ptr<ObjectImplementation>, std::shared_ptr<ArrayImplementation>> schema);

	ValidationResult ValidateFromFile(const std::string& path);//validates a .yaml file using loaded Schema structure
};

