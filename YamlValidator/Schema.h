#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>

#include "Types.h"
#include "YamlParser.h"

using kvPair = std::pair<std::string, ParserTypes::YamlValue>;

class Schema {
private:
public:
	// static void FromFile(std::string filePath); // Om vi har tid?

	Schema(std::vector<kvPair> input) {}

	//ValidationResult Validate(std::string input);
	//ValidationResult ValidateFromFile(std::string fileName);
};

