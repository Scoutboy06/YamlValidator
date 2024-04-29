#include "Types.h"
#include "Schema.h"

#include <typeinfo>

#define ValidationTypeMismatch ValidationResult(ValidationResult::ValidationError(ValidationResult::ValidationError::TypeMismatch));
#define ValidationUnexpectedValue ValidationResult(ValidationResult::ValidationError(ValidationResult::ValidationError::UnexpectedValue));
#define ValidationUnknownError ValidationResult(ValidationResult::ValidationError(ValidationResult::ValidationError::UnknownError));
#define ValidationSuccess ValidationResult(ValidationResult::ValidationSuccess())

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
//
//bool compareTypeToParserType(Schema::Types type, Schema::T yamlInstance)

Schema::ValidationResult Schema::ValidateFromFile(std::string path)
{
	//std::ifstream schemaFile(path);
	//YamlParser parser(schemaFile);
	//ParserResult result = parser.Parse();

	/*ParserTypes::Array yamlArray;

	yamlArray.PushBack(ParserTypes::String("value"));

	Yaml yaml = std::make_shared<ParserTypes::Array>(std::move(yamlArray));*/

	ParserTypes::Object yamlObjectBase;


	ParserTypes::Object yamlObject;

	yamlObject.Set({ "test", ParserTypes::String("test") });
	yamlObject.Set({ "red", ParserTypes::Null() });

	yamlObjectBase.Set({ "test", std::make_shared<ParserTypes::Object>(std::move(yamlObject)) });
	yamlObjectBase.Set({ "test2", ParserTypes::Boolean(true) });

	ParserTypes::Yaml yaml = std::make_shared<ParserTypes::Object>(std::move(yamlObjectBase));


	ParserResult result = ParserResult(yaml);

	if (std::optional<ParserError> errorOptional = result.GetIfError()) {
		if (errorOptional.has_value()) {
			return ValidationResult(ValidationResult::ValidationError(errorOptional.value()));
		}
	}

	if (std::optional<ParserTypes::Yaml> yamlOptional = result.GetIfOk()) {
		if (yamlOptional.has_value()) {
			ParserTypes::Yaml yaml = yamlOptional.value();

			return Schema::Validate(yaml, this->schema);
		}
	}

	return ValidationUnknownError;
}

Schema::ValidationResult Schema::Validate(ParserTypes::Yaml yaml, std::variant<std::shared_ptr<ObjectImplementation>, std::shared_ptr<ArrayImplementation>> schema)
{
	if (std::holds_alternative<std::shared_ptr<ParserTypes::Object>>(yaml)) {
		//yaml base is an object
		
		std::shared_ptr<ParserTypes::Object> yamlObject = std::get<std::shared_ptr<ParserTypes::Object>>(yaml);

		if (!std::holds_alternative<std::shared_ptr<ObjectImplementation>>(schema)) {
			return ValidationUnexpectedValue; //error or something because the base of the schema is not an object while the yaml is which almost certainly means that the yaml does not include schema things which are by default required
		}

		std::shared_ptr<ObjectImplementation> schemaObject = std::get<std::shared_ptr<ObjectImplementation>>(schema);

		std::vector<std::string> yamlObjectKeys = yamlObject->ExtractKeys();

		std::vector<std::string> schemaObjectKeys = schemaObject->ExtractKeys();

		for (std::string yamlObjectKey : yamlObjectKeys) {
			if (!schemaObject->ContainsKey(yamlObjectKey))
				continue;

			std::optional<YamlValue> yamlObjectOptionalValue = yamlObject->Get(yamlObjectKey);

			std::optional<SchemaValue> schemaObjectOptionalValue = schemaObject->Get(yamlObjectKey);

			if (!yamlObjectOptionalValue.has_value() || !schemaObjectOptionalValue.has_value()) {
				return ValidationUnknownError;
			}

			YamlValue yamlObjectValue = yamlObjectOptionalValue.value();

			SchemaValue schemaObjectValue = schemaObjectOptionalValue.value();

			if (std::holds_alternative<std::shared_ptr<ObjectImplementation>>(schemaObjectValue)) {
				//recursion! (object)

				//yamlObjectValue also needs to be of object type or the types don't match
				if (!std::holds_alternative<std::shared_ptr<ParserTypes::Object>>(yamlObjectValue)) {
					return ValidationUnexpectedValue;
				}

				//recursion goes here
				ValidationResult result = Schema::Validate(std::get<std::shared_ptr<ParserTypes::Object>>(yamlObjectValue), std::get<std::shared_ptr<ObjectImplementation>>(schemaObjectValue));

				//if we get an error return, otherwise the object is good and we can check the other keys
				if (std::holds_alternative<ValidationResult::ValidationError>(result.result))
					return result;
			}
			else if (std::holds_alternative<std::shared_ptr<ArrayImplementation>>(schemaObjectValue)) {
				//recursion! (array)

				if (!std::holds_alternative<std::shared_ptr<ParserTypes::Array>>(yamlObjectValue)) {
					return ValidationUnexpectedValue;
				}

				//recursion goes here
				ValidationResult result = Schema::Validate(std::get<std::shared_ptr<ParserTypes::Array>>(yamlObjectValue), std::get<std::shared_ptr<ArrayImplementation>>(schemaObjectValue));

				if (std::holds_alternative<ValidationResult::ValidationError>(result.result))
					return result;

			}
			else if (std::holds_alternative<Either>(schemaObjectValue)) {
				if (!Schema::compareTypeToParserType(std::get<Either>(schemaObjectValue), yamlObjectValue)) { //the object value does not have the right type
					return ValidationTypeMismatch;
				};
			}
			else if (!Schema::compareTypeToParserType(std::get<Types>(schemaObjectValue), yamlObjectValue)) { //the object value does not have the right type
				return ValidationTypeMismatch;
			};
		}


		return ValidationSuccess;
	}

	//yaml base is an array

	if (!std::holds_alternative<std::shared_ptr<ArrayImplementation>>(schema)) {
		//error or something because the base of the schema is not an array while the yaml is which almost certainly means that the yaml does not include schema things which are by default required
		return ValidationUnexpectedValue;
	}

	std::shared_ptr<ArrayImplementation> schemaArray = std::get<std::shared_ptr<ArrayImplementation>>(schema);
	std::variant<Types, Either> schemaArrayType = schemaArray->type; //the type that each element of the array needs to be. (can't be object or array)

	std::shared_ptr<ParserTypes::Array> yamlArray = std::get<std::shared_ptr<ParserTypes::Array>>(yaml);

	for (int i = 0; i < yamlArray->Size(); i++) {
		std::optional<ParserTypes::YamlValue> yamlArrayValueOptional = yamlArray->Get(i);

		if (yamlArrayValueOptional.has_value()) {
			ParserTypes::YamlValue yamlArrayValue = yamlArrayValueOptional.value();

			//array and object are not allowed for now.
			if (std::holds_alternative<std::shared_ptr<ParserTypes::Object>>(yamlArrayValue) || std::holds_alternative<std::shared_ptr<ParserTypes::Array>>(yamlArrayValue)) {
				return ValidationUnexpectedValue;
			}

			//check that the element has the right type.
			if (!Schema::compareTypeToParserType(schemaArrayType, yamlArrayValue)) { //the element does not have the right type
				return ValidationTypeMismatch;
			};

		}
		else { //should never happen but still needs to be handled
			return ValidationUnknownError;
		}
	}

	return ValidationSuccess;
}