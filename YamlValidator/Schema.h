#pragma once

#include <string>
#include <map>
#include <vector>
#include <variant>

#include "Types.h"
#include "YamlParser.h"

using kvPair = std::tuple<std::string, YamlValue, bool>; //std::get<n>(tuple)

class Schema {
private:
	class SchemaStructure {

	};
public:
	// static void FromFile(std::string filePath); // Om vi har tid?

	Schema(std::vector<kvPair> input) {}

	static void FromFile(std::string fileName); //loads schema from file
	/* YAML-file example
	media: { type: string, required: true }
	content:
		- name: { type: string, required: true }
		- label: { type: string, required: true }
		- type: { type: string, required: true }
	*/

	ValidationResult Validate(std::string input);
	ValidationResult ValidateFromFile(std::string path);//validates a .yaml file using loaded Schema structure
};

