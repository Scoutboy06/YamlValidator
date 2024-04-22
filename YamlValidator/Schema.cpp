#include "Types.h"
#include "Schema.h"

std::vector <Schema::SchemaPair> YamlToSchema(ParserTypes::Yaml yaml) {
	if (std::holds_alternative<std::shared_ptr<ParserTypes::Object>>(yaml)) {
		std::shared_ptr<ParserTypes::Object> object = std::get<std::shared_ptr<ParserTypes::Object>>(yaml);

		std::vector<std::string> keys = object->ExtractKeys();

		for (std::string key : keys) {        // use auto for more complex types
			std::optional<ParserTypes::YamlValue> keyValue = object->Get(key);

			if (keyValue.has_value()) {
				ParserTypes::YamlValue value = keyValue.value();
				//if it is an object or array, recursion!

				if (std::holds_alternative<std::shared_ptr<ParserTypes::Object>>(value)) {
					//schemaObject.push_back(YamlToSchema) //make it able to return object
				}

				if (std::holds_alternative<std::shared_ptr<ParserTypes::Array>>(value)) {
					//schemaObject.push_back(YamlToSchema)
				}

				//...

			}
			else {	
				//this will never happen :-)
			}
		}

		//return;
	}

	std::shared_ptr<ParserTypes::Array> object = std::get<std::shared_ptr<ParserTypes::Array>>(yaml);

	return { {":=", Schema::Boolean} };
}

Schema Schema::FromFile(std::string path)
{
	std::ifstream schemaFile(path);
	YamlParser parser(schemaFile);
	ParserResult result = parser.Parse();

	if (std::optional<ParserError> error = result.GetIfError()) {
		//return false;
	}

	if (std::optional<ParserTypes::Yaml> yaml = result.GetIfOk()) {
		if (yaml.has_value()) {
			//YamlToSchema(yaml.value());
		}
	}

	return Schema({});
}

//Schema::ValidationResult Schema::Validate(std::string input)
//{
//	return ValidationResult();
//}

template <typename T>
bool compareTypeToParserType(Schema::Types type, T yamlInstance) {
	if (type == String && std::is_same<T, ParserTypes::String>::value)
		return true;
	else if (type == Number && std::is_same<T, ParserTypes::Number>::value)
		return true;
	else if (type == Boolean && std::is_same<T, ParserTypes::Boolean>::value)
		return true;
	else if (type == Null && std::is_same<T, ParserTypes::Null>::value)
		return true;

	return false;
}

Schema::ValidationResult Schema::ValidateFromFile(std::string path)
{
	std::ifstream schemaFile(path);
	YamlParser parser(schemaFile);
	ParserResult result = parser.Parse();

	if (std::optional<ParserError> error = result.GetIfError()) {
		return ValidationResult();
	}

	if (std::optional<ParserTypes::Yaml> yamlOptional = result.GetIfOk()) {
		if (yamlOptional.has_value()) {
			std::vector<SchemaPair> schema;

			ParserTypes::Yaml yaml = yamlOptional.value();

			if (std::holds_alternative<std::shared_ptr<ParserTypes::Object>>(yaml)) {
				std::shared_ptr<ParserTypes::Object> yamlObject = std::get<std::shared_ptr<ParserTypes::Object>>(yaml);

				return ValidationResult();
			}

			//yaml base is an array

			if (!std::holds_alternative<ArrayImplementation>(this->schema)) {
				return ValidationResult(); //error or something because the base of the schema is not an array while the yaml is which almost certainly means that the yaml does not include schema things which are by default required
			}

			ArrayImplementation schemaArray = std::get<ArrayImplementation>(this->schema);
			Types schemaArrayType = schemaArray.type; //the type that each element of the array needs to be. (can't be object or array)

			std::shared_ptr<ParserTypes::Array> yamlArray = std::get<std::shared_ptr<ParserTypes::Array>>(yaml);

			for (int i = 0; i < yamlArray->Size(); i++) {
				std::optional<ParserTypes::YamlValue> yamlArrayValueOptional = yamlArray->Get(i);

				if (yamlArrayValueOptional.has_value()) {
					ParserTypes::YamlValue yamlArrayValue = yamlArrayValueOptional.value();

					//array and object are not allowed for now.
					if (std::holds_alternative<std::shared_ptr<ParserTypes::Object>>(yamlArrayValue) || std::holds_alternative<std::shared_ptr<ParserTypes::Array>>(yamlArrayValue)) {
						return ValidationResult();
					}

					if (!compareTypeToParserType(schemaArrayType, yamlArrayValue)) { //the element does not have the right type
						return ValidationResult();
					};

					//... -> check that the element has the right type.
				}
				else { // 
					return ValidationResult();
				}
			}


			//YamlToSchema(yaml.value());
		}
	}

	return ValidationResult();
}