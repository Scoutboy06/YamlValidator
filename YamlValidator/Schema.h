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



	enum Types {
		String,
		Number,
		Boolean,
		Null
	};

	struct ObjectImplementation;

	//struct Either;

	struct Array {
	private:
		Types type;
	public:
		Array(Types type) : type(type) {};
	};

	struct Either {
		std::vector<Types> values;

		template<typename... Args>
		Either(Args... args) : values({args...}) {
			
		};
	};

	using SchemaValue = std::variant<Types, Either, Array, std::shared_ptr<ObjectImplementation>>;
	// SchemaValue needs a shared pointer for Object because it is forward declared
	// and std::variant typically needs to know the objects size.


	struct ObjectImplementation {
	private:
		std::map<std::string,SchemaValue> values;
	public:
		ObjectImplementation(std::map<std::string, SchemaValue> values) : values(values) {};
	};

	static std::shared_ptr<ObjectImplementation> Object(std::map<std::string, SchemaValue> values) {
		return std::make_shared<ObjectImplementation>(ObjectImplementation(values));
	};


	using SchemaPair = std::pair<std::string, SchemaValue>;


	// static void FromFile(std::string filePath); // Om vi har tid?

	Schema(std::vector<SchemaPair> input) {
		
	}

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

