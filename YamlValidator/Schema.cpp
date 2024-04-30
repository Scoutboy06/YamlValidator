#include "Types.h"
#include "Schema.h"

#include <typeinfo>

#define ValidationTypeMismatch ValidationResult(ValidationResult::ValidationError(ValidationResult::ValidationError::TypeMismatch));
#define ValidationUnexpectedValue ValidationResult(ValidationResult::ValidationError(ValidationResult::ValidationError::UnexpectedValue));
#define ValidationUnknownError ValidationResult(ValidationResult::ValidationError(ValidationResult::ValidationError::UnknownError));
#define ValidationSuccess ValidationResult(ValidationResult::ValidationSuccess())

std::vector <Schema::SchemaPair> YamlToSchema(parser_types::Yaml yaml) {
	if (std::holds_alternative<std::shared_ptr<parser_types::Object>>(yaml)) {
		std::shared_ptr<parser_types::Object> object = std::get<std::shared_ptr<parser_types::Object>>(yaml);

		std::vector<std::string> keys = object->ExtractKeys();

		for (std::string& key : keys) {        // use auto for more complex types
			std::optional<parser_types::YamlValue> keyValue = object->Get(key);

			if (keyValue.has_value()) {
				parser_types::YamlValue value = keyValue.value();
				//if it is an object or array, recursion!

				if (std::holds_alternative<std::shared_ptr<parser_types::Object>>(value)) {
					//schemaObject.push_back(YamlToSchema) //make it able to return object
				}

				if (std::holds_alternative<std::shared_ptr<parser_types::Array>>(value)) {
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

	std::shared_ptr<parser_types::Array> object = std::get<std::shared_ptr<parser_types::Array>>(yaml);

	return { {":=", Schema::Boolean} };
}

Schema Schema::FromFile(const std::string& path)
{
	std::ifstream schemaFile(path);
	YamlParser parser(schemaFile);
	ParserResult result = parser.Parse();

	if (std::optional<ParserError> error = result.GetIfError()) {
		//return false;
	}

	if (std::optional<parser_types::Yaml> yaml = result.GetIfOk()) {
		if (yaml.has_value()) {
			//YamlToSchema(yaml.value());
		}
	}

	return Schema({});
}

Schema::ValidationResult Schema::ValidateFromFile(const std::string& path)
{

	/*parser_types::Array yamlArray;

	yamlArray.PushBack(parser_types::String("value"));
	yamlArray.PushBack(parser_types::Boolean("value"));

	parser_types::Object yamlObjectBase;


	parser_types::Object yamlObject;

	yamlObject.Set({ "test", parser_types::String("test") });
	yamlObject.Set({ "red", std::make_shared<parser_types::Array>(std::move(yamlArray)) });

	yamlObjectBase.Set({ "test", std::make_shared<parser_types::Object>(std::move(yamlObject)) });
	yamlObjectBase.Set({ "test2", parser_types::Boolean(true) });

	parser_types::Yaml yaml = std::make_shared<parser_types::Object>(std::move(yamlObjectBase));


	ParserResult result = ParserResult(yaml);*/

	ParserResult result = ParseYaml(path);

	if (std::optional<ParserError> errorOptional = result.GetIfError()) {
		if (errorOptional.has_value()) {
			return ValidationResult(ValidationResult::ValidationError(errorOptional.value()));
		}
	}

	if (std::optional<parser_types::Yaml> yamlOptional = result.GetIfOk()) {
		if (yamlOptional.has_value()) {
			parser_types::Yaml yaml = yamlOptional.value();

			return Schema::Validate(yaml, this->schema);
		}
	}

	return ValidationUnknownError;
}

Schema::ValidationResult Schema::Validate(
	parser_types::Yaml yaml,
	std::variant<std::shared_ptr<ObjectImplementation>, std::shared_ptr<ArrayImplementation>> schema
)
{
	// 'yaml' is of type Object:
	if (std::holds_alternative<std::shared_ptr<parser_types::Object>>(yaml)) {
		if (!std::holds_alternative<std::shared_ptr<ObjectImplementation>>(schema)) {
			return ValidationUnexpectedValue; //error or something because the base of the schema is not an object while the yaml is which almost certainly means that the yaml does not include schema things which are by default required
		}
		
		std::shared_ptr<parser_types::Object> yamlObject = std::get<std::shared_ptr<parser_types::Object>>(yaml);
		std::shared_ptr<ObjectImplementation> schemaObject = std::get<std::shared_ptr<ObjectImplementation>>(schema);

		std::vector<std::string> yamlObjectKeys = yamlObject->ExtractKeys();
		std::vector<std::string> schemaObjectKeys = schemaObject->ExtractKeys();

		for (std::string& yamlObjectKey : yamlObjectKeys) {
			if (!schemaObject->ContainsKey(yamlObjectKey))
				continue;

			std::optional<YamlValue> yamlObjectOptionalValue = yamlObject->Get(yamlObjectKey);
			std::optional<SchemaValue> schemaObjectOptionalValue = schemaObject->Get(yamlObjectKey);

			if (!yamlObjectOptionalValue.has_value() || !schemaObjectOptionalValue.has_value())
				return ValidationUnknownError;

			YamlValue yamlObjectValue = yamlObjectOptionalValue.value();
			SchemaValue schemaObjectValue = schemaObjectOptionalValue.value();

			if (std::holds_alternative<std::shared_ptr<ObjectImplementation>>(schemaObjectValue)) {
				//recursion! (object)

				//yamlObjectValue also needs to be of object type or the types don't match
				if (!std::holds_alternative<std::shared_ptr<parser_types::Object>>(yamlObjectValue))
					return ValidationUnexpectedValue;

				//recursion goes here
				ValidationResult result = Schema::Validate(
					std::get<std::shared_ptr<parser_types::Object>>(yamlObjectValue),
					std::get<std::shared_ptr<ObjectImplementation>>(schemaObjectValue)
				);

				//if we get an error return, otherwise the object is good and we can check the other keys
				if (std::holds_alternative<ValidationResult::ValidationError>(result.result))
					return result;
			}

			else if (std::holds_alternative<std::shared_ptr<ArrayImplementation>>(schemaObjectValue)) {
				//recursion! (array)

				if (!std::holds_alternative<std::shared_ptr<parser_types::Array>>(yamlObjectValue))
					return ValidationUnexpectedValue;

				//recursion goes here
				ValidationResult result = Schema::Validate(
					std::get<std::shared_ptr<parser_types::Array>>(yamlObjectValue),
					std::get<std::shared_ptr<ArrayImplementation>>(schemaObjectValue)
				);

				if (std::holds_alternative<ValidationResult::ValidationError>(result.result))
					return result;
			}

			else if (std::holds_alternative<Either>(schemaObjectValue)) {
				if (!Schema::compareTypeToParserType(std::get<Either>(schemaObjectValue), yamlObjectValue)) { //the object value does not have the right type
					return ValidationTypeMismatch;
				}
			}
			
			else if (!Schema::compareTypeToParserType(std::get<Types>(schemaObjectValue), yamlObjectValue)) { //the object value does not have the right type
				return ValidationTypeMismatch;
			}
		}


		return ValidationSuccess;
	}

	// 'yaml' MUST be of type Array

	// If 'yaml' somehow isn't of type Array, throw an error
	if (!std::holds_alternative<std::shared_ptr<ArrayImplementation>>(schema))
		return ValidationUnexpectedValue;

	std::shared_ptr<ArrayImplementation> schemaArray = std::get<std::shared_ptr<ArrayImplementation>>(schema);
	std::variant<Types, Either> schemaArrayType = schemaArray->type; //the type that each element of the array needs to be. (can't be object or array)

	std::shared_ptr<parser_types::Array> yamlArray = std::get<std::shared_ptr<parser_types::Array>>(yaml);

	for (int i = 0; i < yamlArray->Size(); i++) {
		std::optional<parser_types::YamlValue> yamlArrayValueOptional = yamlArray->Get(i);

		if (yamlArrayValueOptional.has_value()) {
			parser_types::YamlValue yamlArrayValue = yamlArrayValueOptional.value();

			//array and object are not allowed for now.
			if (std::holds_alternative<std::shared_ptr<parser_types::Object>>(yamlArrayValue) ||
				std::holds_alternative<std::shared_ptr<parser_types::Array>>(yamlArrayValue)
			) {
				return ValidationUnexpectedValue;
			}

			//check that the element has the right type.
			if (!Schema::compareTypeToParserType(schemaArrayType, yamlArrayValue)) //the element does not have the right type
				return ValidationTypeMismatch;
		}

		else { //should never happen but still needs to be handled
			return ValidationUnknownError;
		}
	}

	return ValidationSuccess;
}