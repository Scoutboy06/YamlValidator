#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <typeinfo>

#include "Types.h"
#include "YamlParser.h"

class Schema {
public:
	struct ValidationResult {
		struct ValidationError {
			enum SchemaError {
				TypeMismatch,
				UnexpectedValue,
				UnknownError
			};

			/*int column;
			int row;*/

			std::variant<ParserError, SchemaError> error;

			ValidationError(std::variant<ParserError, SchemaError> error/*, int column = -1, int row = -1*/) : error(error)/*, column(column), row(row)*/ {};
		};

		struct ValidationSuccess {

		};

		std::variant<ValidationError, ValidationSuccess> result;

		ValidationResult(std::variant<ValidationError, ValidationSuccess> result) : result(result) { }
	};

	enum Types {
		String,
		Number,
		Boolean,
		Null
	};


	struct ObjectImplementation; //needs to be declared here so it can be forward declared later.

	struct Either {
		std::vector<Types> values;

		template<typename... Args>
		Either(Args... args) : values({args...}) { };
	};

	struct ArrayImplementation {
	public:
		std::variant<Types, Either> type;
		ArrayImplementation(std::variant<Types, Either> type) : type(type) {};
	};


	using SchemaValue = std::variant<Types, Either, std::shared_ptr<ArrayImplementation>, std::shared_ptr<ObjectImplementation>>;
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

	static std::shared_ptr<ArrayImplementation> CreateArray(std::variant<Types, Either> values) {
		return std::make_shared<ArrayImplementation>(ArrayImplementation(values));
	};

	static std::shared_ptr<ObjectImplementation> CreateObject(std::unordered_map<std::string, SchemaValue> values) {
		return std::make_shared<ObjectImplementation>(ObjectImplementation(values));
	};


	using SchemaPair = std::pair<std::string, SchemaValue>;

private:
	std::variant<std::shared_ptr<ObjectImplementation>, std::shared_ptr<ArrayImplementation>> schema;
	Schema YamlToSchema(parser_types::Yaml yaml) {};

	static bool compareTypeToParserType(std::variant<Types,Either> type, parser_types::YamlValue yamlInstance) {
		if (std::holds_alternative<Either>(type)) {
			Either eitherType = std::get<Either>(type);

			for (const Types& typesType : eitherType.values) {
				if (compareTypeToParserType(typesType, yamlInstance))
					return true;
			}

			return false;
		}

		Types typesType = std::get<Types>(type);

		if (typesType == Schema::String && std::holds_alternative<parser_types::String>(yamlInstance))
			return true;
		else if (typesType == Schema::Number && std::holds_alternative<parser_types::Number>(yamlInstance))
			return true;
		else if (typesType == Schema::Boolean && std::holds_alternative<parser_types::Boolean>(yamlInstance))
			return true;
		else if (typesType == Schema::Null && std::holds_alternative<parser_types::Null>(yamlInstance))
			return true;

		return false;
	}
public:
	// static void FromFile(std::string filePath); // Om vi har tid?

	Schema(std::variant<std::shared_ptr<ObjectImplementation>, std::shared_ptr<ArrayImplementation>> schema) : schema(schema) {
		std::cout << "ab" << std::endl;
	};

	static Schema FromFile(const std::string& path);
	//loads schema from file
	/* YAML-file example
	media: { type: string, required: true }
	content:
		- name: { type: string, required: true }
		- label: { type: string, required: true }
		- type: { type: string, required: true }

		list: [1, 0, 2]
		object:
			currency:  [USD, 2]
			value: 42.99

		array:
		  - [currency, USD, 45]
		  - [value, 42.99]
	*/

	//ValidationResult Validate(std::string input);

	static ValidationResult Validate(parser_types::Yaml yaml, std::variant<std::shared_ptr<ObjectImplementation>, std::shared_ptr<ArrayImplementation>> schema);

	ValidationResult ValidateFromFile(const std::string& path);//validates a .yaml file using loaded Schema structure
};

