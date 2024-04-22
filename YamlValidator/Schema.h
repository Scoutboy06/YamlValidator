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

	};

	enum Types {
		String,
		Number,
		Boolean,
		Null
	};


	struct ObjectImplementation;

	//struct Either;

	struct ArrayImplementation {
	public:
		Types type;
		ArrayImplementation(Types type) : type(type) {};
	};

	struct Either {
		std::vector<Types> values;

		template<typename... Args>
		Either(Args... args) : values({args...}) { };
	};

	using SchemaValue = std::variant<Types, Either, ArrayImplementation, std::shared_ptr<ObjectImplementation>>;
	// SchemaValue needs a shared pointer for Object because it is forward declared
	// and std::variant typically needs to know the objects size at declaration.


	struct ObjectImplementation {
	public:
		std::unordered_map<std::string,SchemaValue> values;
		ObjectImplementation(std::unordered_map<std::string, SchemaValue> values) : values(values) {};
	};

	static ArrayImplementation CreateArray(ArrayImplementation values) {
		return ArrayImplementation(values);
	};

	static std::shared_ptr<ObjectImplementation> CreateObject(std::unordered_map<std::string, SchemaValue> values) {
		return std::make_shared<ObjectImplementation>(ObjectImplementation(values));
	};


	using SchemaPair = std::pair<std::string, SchemaValue>;

private:
	std::variant<std::unordered_map<std::string, SchemaValue>, ArrayImplementation> schema;
	Schema YamlToSchema(ParserTypes::Yaml yaml) {};
	template <typename T>
	bool compareTypeToParserType(Types type, T yamlInstance);
public:
	// static void FromFile(std::string filePath); // Om vi har tid?

	Schema(std::variant<std::unordered_map<std::string, SchemaValue>, ArrayImplementation> schema) : schema(schema) {
		std::cout << "ab" << std::endl;
	};

	static Schema FromFile(std::string path);
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
	ValidationResult ValidateFromFile(std::string path);//validates a .yaml file using loaded Schema structure
};

