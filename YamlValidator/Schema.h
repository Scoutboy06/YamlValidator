#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>

#include "Types.h"
#include "YamlParser.h"

using kvPair = std::pair<std::string, YamlValue>;

// Ok and Error defined in YamlParser.h
// TODO: move Ok and Error to global space?
using ValidationResult = std::variant<Ok, Error>;

class Schema {
private:
public:
	// static void FromFile(std::string filePath); // Om vi har tid?

	Schema(std::vector<kvPair> input) {}

	ValidationResult Validate(std::string input);
	ValidationResult ValidateFromFile(std::string fileName);
};

