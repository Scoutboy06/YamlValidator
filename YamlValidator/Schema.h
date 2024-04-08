#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>

#include "YamlParser.h"

// The type values for defining a Schema
enum YamlType {
	String,
	Number,
	Boolean,
	Null,
};

class Object;
class Array;

using kvPair = std::pair<std::string, std::variant<YamlType, Array, Object>>;

class Array {
	Array(YamlType input) {}
	Array(std::initializer_list<kvPair> input) {}
};

class Object {
	Object(kvPair input[]) {}
	Object(std::initializer_list<kvPair> input) {}
};

class Schema {
private:
public:
	struct Ok;
	struct Error {
		std::string message;
		Error(std::string message) : message(message) {}
	};

	using ValidationResult = std::variant<Ok, Error>;

	// static void FromFile(std::string filePath); // Om vi har tid?

	Schema(std::vector<kvPair> input) {}

	ValidationResult Validate(std::string input);
	ValidationResult ValidateFromFile(std::string fileName);
};

