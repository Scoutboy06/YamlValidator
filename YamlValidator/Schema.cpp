#include "Types.h"
#include "Schema.h"

#include <typeinfo>

std::string Schema::getTypeName(std::variant<Type, Either, parser_types::YamlValue> instance) {
    if (std::holds_alternative<parser_types::YamlValue>(instance)) {
        parser_types::YamlValue yamlValueInstance = std::get<parser_types::YamlValue>(instance);

        if (std::holds_alternative<parser_types::String>(yamlValueInstance))
            return "String";
        else if (std::holds_alternative<parser_types::Number>(yamlValueInstance))
            return "Number";
        else if (std::holds_alternative<parser_types::Boolean>(yamlValueInstance))
            return "Boolean";
        else if (std::holds_alternative<parser_types::Null>(yamlValueInstance))
            return "Null";
        else if (std::holds_alternative<parser_types::Timestamp>(yamlValueInstance))
            return "Timestamp";
        else if (std::holds_alternative<std::shared_ptr<parser_types::Object>>(yamlValueInstance))
            return "Object";
        else if (std::holds_alternative<std::shared_ptr<parser_types::Array>>(yamlValueInstance))
            return "Array";

        return "";
    }
    else if (std::holds_alternative<Either>(instance)) {
        Either eitherInstance = std::get<Either>(instance);

        std::string name = "Either<";

        for (Type& eitherInstanceType : eitherInstance.values) {
            name += getTypeName(eitherInstanceType) + ",";
        }

        name.pop_back(); //remove trailing comma

        name += ">";

        return name;
    }

    Type typeInstance = std::get<Type>(instance);

    if (typeInstance == String)
        return "String";
    else if (typeInstance == Number)
        return "Number";
    else if (typeInstance == Boolean)
        return "Boolean";
    else if (typeInstance == Null)
        return "Null";
    else if (typeInstance == Timestamp)
        return "Timestamp";

    return "";
};

bool Schema::compareTypeToParserType(std::variant<Type, Either> type, parser_types::YamlValue yamlInstance) {
    if (std::holds_alternative<Either>(type)) {
        Either eitherType = std::get<Either>(type);

        for (const Type& typesType : eitherType.values) {
            if (compareTypeToParserType(typesType, yamlInstance))
                return true;
        }

        return false;
    }

    Type typesType = std::get<Type>(type);

    if (typesType == Schema::String && std::holds_alternative<parser_types::String>(yamlInstance))
        return true;
    else if (typesType == Schema::Number && std::holds_alternative<parser_types::Number>(yamlInstance))
        return true;
    else if (typesType == Schema::Boolean && std::holds_alternative<parser_types::Boolean>(yamlInstance))
        return true;
    else if (typesType == Schema::Null && std::holds_alternative<parser_types::Null>(yamlInstance))
        return true;
    else if (typesType == Schema::Timestamp && std::holds_alternative<parser_types::Timestamp>(yamlInstance))
        return true;

    return false;
};

Schema::ValidationResult Schema::GetValidationError(std::optional<std::variant<Schema::SchemaError::ArrayError, SchemaError::ObjectError>> errorInformation, ErrorType errorType, std::string message) {
    return Schema::ValidationResult(Schema::ValidationResult::ValidationError(Schema::SchemaError(errorInformation, errorType, message)));
}

Schema::ValidationResult Schema::GetValidationErrorMismatch(std::optional<std::variant<SchemaError::ArrayError, SchemaError::ObjectError>> errorInformation, std::variant<Type,Either> expected, parser_types::YamlValue got) {
    std::string expectedTypeName = "";
    std::string gotTypeName = getTypeName(got);
    
    if(std::holds_alternative<Type>(expected))
        expectedTypeName = getTypeName(std::get<Type>(expected));
    else
        expectedTypeName = getTypeName(std::get<Either>(expected));

    std::string message = std::format("TypeMismatch: Expected {} but got {}", expectedTypeName, gotTypeName);
    
    return GetValidationError(errorInformation, ErrorType::TypeMismatch, message);
}

Schema::ValidationResult Schema::GetValidationErrorUnexpected(std::optional<std::variant<SchemaError::ArrayError, SchemaError::ObjectError>> errorInformation, parser_types::YamlValue unexpected) {
    std::string unexpectedTypeName = getTypeName(unexpected);

    std::string message = std::format("UnexpectedValue: {}", unexpectedTypeName);

    return GetValidationError(errorInformation, ErrorType::UnexpectedValue, message);
}


#define ValidationSuccess ValidationResult(ValidationResult::ValidationSuccess())

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
                    return GetValidationErrorUnexpected(SchemaError::ObjectError(yamlObject, yamlObjectKey), yamlObjectValue);
                    //return GetValidationError(SchemaError::ObjectError(yamlObject, yamlObjectKey), Schema::ErrorType::UnexpectedValue);  //ValidationUnexpectedValue;

                ValidationResult result = Schema::Validate(std::get<std::shared_ptr<parser_types::Object>>(yamlObjectValue), std::get<std::shared_ptr<ObjectImplementation>>(schemaObjectValue));

                //if we get an error return, otherwise the object is good and we can check the other keys
                if (std::holds_alternative<ValidationResult::ValidationError>(result.result))
                    return result;
            }

            else if (std::holds_alternative<std::shared_ptr<ArrayImplementation>>(schemaObjectValue)) {
                //recursion! (array)

                if (!std::holds_alternative<std::shared_ptr<parser_types::Array>>(yamlObjectValue))
                    return GetValidationErrorUnexpected(SchemaError::ObjectError(yamlObject, yamlObjectKey), yamlObjectValue);

                ValidationResult result = Schema::Validate(std::get<std::shared_ptr<parser_types::Array>>(yamlObjectValue), std::get<std::shared_ptr<ArrayImplementation>>(schemaObjectValue));

                if (std::holds_alternative<ValidationResult::ValidationError>(result.result))
                    return result;
            }
            else if (std::holds_alternative<Either>(schemaObjectValue)) {
                //the object value does not have the right type
                if (!Schema::compareTypeToParserType(std::get<Either>(schemaObjectValue), yamlObjectValue)) 
                    return GetValidationErrorMismatch(SchemaError::ObjectError(yamlObject, yamlObjectKey), std::get<Either>(schemaObjectValue), yamlObjectValue);
            }  //the object value does not have the right type
            else if (!Schema::compareTypeToParserType(std::get<Type>(schemaObjectValue), yamlObjectValue))
                return GetValidationErrorMismatch(SchemaError::ObjectError(yamlObject, yamlObjectKey), std::get<Type>(schemaObjectValue), yamlObjectValue);
        }


        return ValidationSuccess;
    }

    // 'yaml' MUST be of type Array

    if (!std::holds_alternative<std::shared_ptr<ArrayImplementation>>(schema))
        return GetValidationError(std::nullopt, Schema::ErrorType::UnexpectedValue);

    std::shared_ptr<ArrayImplementation> schemaArray = std::get<std::shared_ptr<ArrayImplementation>>(schema);
    std::variant<Type, Either> schemaArrayType = schemaArray->type; //the type that each element of the array needs to be. (can't be object or array)

    std::shared_ptr<parser_types::Array> yamlArray = std::get<std::shared_ptr<parser_types::Array>>(yaml);

    for (int i = 0; i < yamlArray->Size(); i++) {
        std::optional<parser_types::YamlValue> yamlArrayValueOptional = yamlArray->Get(i);

        if (yamlArrayValueOptional.has_value()) {
            parser_types::YamlValue yamlArrayValue = yamlArrayValueOptional.value();

            //array and object are not allowed for now.
            if (std::holds_alternative<std::shared_ptr<parser_types::Object>>(yamlArrayValue) || std::holds_alternative<std::shared_ptr<parser_types::Array>>(yamlArrayValue))
                return GetValidationErrorUnexpected(SchemaError::ArrayError(yamlArray, i), yamlArrayValue);

            //check that the element has the right type.
            if (!Schema::compareTypeToParserType(schemaArrayType, yamlArrayValue)) //the element does not have the right type
                return GetValidationErrorMismatch(SchemaError::ArrayError(yamlArray, i), schemaArrayType, yamlArrayValue);

        }
        else//should never happen but still needs to be handled
            return GetValidationError(std::nullopt, Schema::ErrorType::UnknownError);
    }

    return ValidationSuccess;
}