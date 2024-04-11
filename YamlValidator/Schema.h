#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>

#include "Types.h"
#include "YamlParser.h"

class Schema {
private:
	class SchemaStructure {

	};
public:
	struct ValidationResult {

	};

	enum class Types {
		String,
		Integer,
		Float,
		Boolean,
		Null
	};

	struct Array;
	struct Object;

	using SchemaValue = std::variant<Types, std::shared_ptr<Array>, std::shared_ptr<Object>>;


	struct Array {
	private:
		SchemaValue type;
	public:
		Array(SchemaValue type) : type(type) {};
	};

	struct Object {
	private:
		std::map<std::string, SchemaValue> values;
	public:
		Object(std::map<std::string, SchemaValue> values) : values(values) {};
	};


	using SchemaPair = std::pair<std::string, SchemaValue>;


	// static void FromFile(std::string filePath); // Om vi har tid?

	Schema(std::vector<SchemaPair> input) {}

	static void FromFile(std::string fileName); //loads schema from file
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

	ValidationResult Validate(std::string input);
	ValidationResult ValidateFromFile(std::string path);//validates a .yaml file using loaded Schema structure
};

