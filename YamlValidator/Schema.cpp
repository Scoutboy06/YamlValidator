#include "Types.h"
#include "Schema.h"

#include <typeinfo>

Schema::ValidationResult GetValidationError(std::optional<std::variant<Schema::SchemaError::ArrayError, Schema::SchemaError::ObjectError>> errorInformation, Schema::ErrorType errorType) {
	return Schema::ValidationResult(Schema::ValidationResult::ValidationError(Schema::SchemaError(errorInformation, errorType)));
}

//Schema::ValidationResult GetValidationErrorMismatch(std::optional<std::variant<Schema::SchemaError::ArrayError, Schema::SchemaError::ObjectError>> errorInformation, Schema::ErrorType errorType, Schema::Types expected, Schema::Types got) {
//	std::string message = std::format()
//}


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
	//std::nullopt
	return GetValidationError(std::nullopt, Schema::ErrorType::UnknownError);
}

Schema::ValidationResult Schema::Validate(
	parser_types::Yaml yaml,
	std::variant<std::shared_ptr<ObjectImplementation>, std::shared_ptr<ArrayImplementation>> schema
)
{
	// 'yaml' is of type Object:
	if (std::holds_alternative<std::shared_ptr<parser_types::Object>>(yaml)) {
		if (!std::holds_alternative<std::shared_ptr<ObjectImplementation>>(schema))
			return  GetValidationError(std::nullopt, ErrorType::UnexpectedValue);//error or something because the base of the schema is not an object while the yaml is which almost certainly means that the yaml does not include schema things which are by default required
		
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
				return GetValidationError(SchemaError::ObjectError(yamlObject, yamlObjectKey), Schema::ErrorType::UnknownError); //ValidationUnknownError;

			YamlValue yamlObjectValue = yamlObjectOptionalValue.value();
			SchemaValue schemaObjectValue = schemaObjectOptionalValue.value();

			if (std::holds_alternative<std::shared_ptr<ObjectImplementation>>(schemaObjectValue)) {
				//recursion! (object)

				//yamlObjectValue also needs to be of object type or the types don't match
				if (!std::holds_alternative<std::shared_ptr<parser_types::Object>>(yamlObjectValue))
					return GetValidationError(SchemaError::ObjectError(yamlObject, yamlObjectKey), Schema::ErrorType::UnexpectedValue);  //ValidationUnexpectedValue;

				//recursion goes here
				ValidationResult result = Schema::Validate(std::get<std::shared_ptr<parser_types::Object>>(yamlObjectValue), std::get<std::shared_ptr<ObjectImplementation>>(schemaObjectValue));

				//if we get an error return, otherwise the object is good and we can check the other keys
				if (std::holds_alternative<ValidationResult::ValidationError>(result.result))
					return result;
			}

			else if (std::holds_alternative<std::shared_ptr<ArrayImplementation>>(schemaObjectValue)) {
				//recursion! (array)

				if (!std::holds_alternative<std::shared_ptr<parser_types::Array>>(yamlObjectValue)) {
					//return ValidationUnexpectedValue;
					return GetValidationError(SchemaError::ObjectError(yamlObject, yamlObjectKey), Schema::ErrorType::UnexpectedValue);
				}

				//recursion goes here
				ValidationResult result = Schema::Validate(std::get<std::shared_ptr<parser_types::Array>>(yamlObjectValue), std::get<std::shared_ptr<ArrayImplementation>>(schemaObjectValue));

				if (std::holds_alternative<ValidationResult::ValidationError>(result.result))
					return result;
			}
			else if (std::holds_alternative<Either>(schemaObjectValue)) {
				if (!Schema::compareTypeToParserType(std::get<Either>(schemaObjectValue), yamlObjectValue)) { //the object value does not have the right type
					//return ValidationTypeMismatch;
					return GetValidationError(SchemaError::ObjectError(yamlObject, yamlObjectKey), Schema::ErrorType::TypeMismatch);
				};
			}
			
			else if (!Schema::compareTypeToParserType(std::get<Types>(schemaObjectValue), yamlObjectValue)) { //the object value does not have the right type
				//return ValidationTypeMismatch;
				return GetValidationError(SchemaError::ObjectError(yamlObject, yamlObjectKey), Schema::ErrorType::TypeMismatch);
			};
		}


		return ValidationSuccess;
	}

	// 'yaml' MUST be of type Array

	if (!std::holds_alternative<std::shared_ptr<ArrayImplementation>>(schema))
		return GetValidationError(std::nullopt, Schema::ErrorType::UnexpectedValue);

	std::shared_ptr<ArrayImplementation> schemaArray = std::get<std::shared_ptr<ArrayImplementation>>(schema);
	std::variant<Types, Either> schemaArrayType = schemaArray->type; //the type that each element of the array needs to be. (can't be object or array)

	std::shared_ptr<parser_types::Array> yamlArray = std::get<std::shared_ptr<parser_types::Array>>(yaml);

	for (int i = 0; i < yamlArray->Size(); i++) {
		std::optional<parser_types::YamlValue> yamlArrayValueOptional = yamlArray->Get(i);

		if (yamlArrayValueOptional.has_value()) {
			parser_types::YamlValue yamlArrayValue = yamlArrayValueOptional.value();

			//array and object are not allowed for now.
			if (std::holds_alternative<std::shared_ptr<parser_types::Object>>(yamlArrayValue) || std::holds_alternative<std::shared_ptr<parser_types::Array>>(yamlArrayValue))
				return GetValidationError(SchemaError::ArrayError(yamlArray, i), Schema::ErrorType::UnexpectedValue);

			//check that the element has the right type.
			if (!Schema::compareTypeToParserType(schemaArrayType, yamlArrayValue)) //the element does not have the right type
				return GetValidationError(SchemaError::ArrayError(yamlArray, i), Schema::ErrorType::TypeMismatch);

		}
		else//should never happen but still needs to be handled
			return GetValidationError(std::nullopt, Schema::ErrorType::UnknownError);
	}

	return ValidationSuccess;
}