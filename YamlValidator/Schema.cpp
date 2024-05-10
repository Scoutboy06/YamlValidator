#include "Types.h"
#include "Schema.h"

#include <typeinfo>

std::string Schema::getTypeName(std::variant<SchemaValue, parser_types::YamlValue> instance) {
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
        else if (std::holds_alternative<std::shared_ptr<parser_types::Object>>(yamlValueInstance)) {

            std::shared_ptr<parser_types::Object> yamlValueObject = std::get<std::shared_ptr<parser_types::Object>>(yamlValueInstance);

            std::vector<std::string> keys = yamlValueObject->ExtractKeys();

            std::string name = "Object({";
            for (std::string& key : keys) {
                std::optional<parser_types::YamlValue> valueOptional = yamlValueObject->Get(key);
                if (valueOptional.has_value()) {
                    name += "'" + key + "': ";
                    name += getTypeName(valueOptional.value()) + "},{";
                }
            }

            name.pop_back(); //remove trailing bracket
            name.pop_back(); //remove trailing comma

            return name + "})";
        }
        else if (std::holds_alternative<std::shared_ptr<parser_types::Array>>(yamlValueInstance)) {
            std::shared_ptr<parser_types::Array> yamlValueArray = std::get<std::shared_ptr<parser_types::Array>>(yamlValueInstance);

            std::string name = "Array({";

            for (int i = 0; i < yamlValueArray->Size(); i++) {
                std::optional<parser_types::YamlValue> yamlArrayValueOptional = yamlValueArray->Get(i);

                if (yamlArrayValueOptional.has_value()) {
                    parser_types::YamlValue yamlArrayValue = yamlArrayValueOptional.value();

                    name += getTypeName(yamlArrayValue) + ",";
                }
            }

            name.pop_back(); //remove trailing comma

            return name + "})";
        }

        return "";
    }

    SchemaValue schemaValueInstance = std::get<SchemaValue>(instance);

    if (std::holds_alternative<Either>(schemaValueInstance)) {
        Either eitherInstance = std::get<Either>(schemaValueInstance);

        std::string name = "Either<";

        for (SchemaValue& eitherInstanceType : eitherInstance.values) {
            name += getTypeName(eitherInstanceType) + ",";
        }

        name.pop_back(); //remove trailing comma

        name += ">";

        return name;
    }
    else if (std::holds_alternative<Type>(schemaValueInstance)) {
        Type typeInstance = std::get<Type>(schemaValueInstance);

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
    }
    else if (std::holds_alternative<std::shared_ptr<ObjectImplementation>>(schemaValueInstance)) {

        std::shared_ptr<ObjectImplementation> schemaValueObject = std::get<std::shared_ptr<ObjectImplementation>>(schemaValueInstance);

        std::vector<std::string> keys = schemaValueObject->ExtractKeys();

        std::string name = "Object({";
        for (std::string& key : keys) {
            std::optional<SchemaValue> valueOptional = schemaValueObject->Get(key);
            if (valueOptional.has_value()) {
                name += "'" + key + "': ";
                name += getTypeName(valueOptional.value()) + "},{";
            }
        }

        name.pop_back(); //remove trailing bracket
        name.pop_back(); //remove trailing comma

        return name + ")";
    } 
    else if (std::holds_alternative<std::shared_ptr<ArrayImplementation>>(schemaValueInstance)) {
        std::shared_ptr<ArrayImplementation> schemaValueArray = std::get<std::shared_ptr<ArrayImplementation>>(schemaValueInstance);

        return "Array(" + getTypeName(schemaValueArray->type) + ")";
    }


    return "";
};

bool Schema::compareTypeToParserType(SchemaValue type, parser_types::YamlValue yamlInstance) {
    if (std::holds_alternative<Either>(type)) {
        Either eitherType = std::get<Either>(type);

        for (const SchemaValue& typesType : eitherType.values) {
            if (compareTypeToParserType(typesType, yamlInstance))
                return true;
        }

        return false;
    }
    else if (std::holds_alternative<std::shared_ptr<ObjectImplementation>>(type)) {
        if (std::holds_alternative<std::shared_ptr<parser_types::Object>>(yamlInstance))
            return true;
    }
    else if (std::holds_alternative<std::shared_ptr<ArrayImplementation>>(type)) {
        if (std::holds_alternative<std::shared_ptr<parser_types::Array>>(yamlInstance))
            return true;
    }
    else if (std::holds_alternative<Type>(type)) {
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
    }


    return false;
};

Schema::ValidationResult Schema::GetValidationError(std::optional<std::variant<Schema::SchemaError::ArrayError, SchemaError::ObjectError>> errorInformation, ErrorType errorType, std::string message) {
    return Schema::ValidationResult(Schema::ValidationResult::ValidationError(Schema::SchemaError(errorInformation, errorType, message)));
}

Schema::ValidationResult Schema::GetValidationErrorMismatch(std::optional<std::variant<SchemaError::ArrayError, SchemaError::ObjectError>> errorInformation, SchemaValue expected, parser_types::YamlValue got) {
    std::string expectedTypeName = getTypeName(expected);
    std::string gotTypeName = getTypeName(got);

    std::string message = std::format("TypeMismatch: Expected {} but got {}", expectedTypeName, gotTypeName);
    
    if (errorInformation.has_value()) {
        std::variant<SchemaError::ArrayError, SchemaError::ObjectError> errorInformationValue = errorInformation.value();

        std::string at = "Null";

        parser_types::YamlValue errorRoot = Null;

        if (std::holds_alternative<SchemaError::ArrayError>(errorInformation.value())) {
            SchemaError::ArrayError errorInformationArray = std::get<SchemaError::ArrayError>(errorInformationValue);

            errorRoot = errorInformationArray.errorRoot;


            if(errorInformationArray.index.has_value())
                at = std::to_string(errorInformationArray.index.value());

        }
        else {
            SchemaError::ObjectError errorInformationObject = std::get<SchemaError::ObjectError>(errorInformationValue);

            errorRoot = errorInformationObject.errorRoot;

            if (errorInformationObject.key.has_value())
                at = errorInformationObject.key.value();
        }
        
        message += std::format(" at '{}' in {}", at, getTypeName(errorRoot));
    }

    
    return GetValidationError(errorInformation, ErrorType::TypeMismatch, message);
}

Schema::ValidationResult Schema::GetValidationErrorUnexpected(std::optional<std::variant<SchemaError::ArrayError, SchemaError::ObjectError>> errorInformation, parser_types::YamlValue unexpected) {
    std::string unexpectedTypeName = getTypeName(unexpected);

    std::string message = std::format("UnexpectedValue: {}", unexpectedTypeName);

    return GetValidationError(errorInformation, ErrorType::UnexpectedValue, message);
}


#define GetValidationSuccess ValidationResult(ValidationResult::ValidationSuccess())

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

    return GetValidationError(std::nullopt, Schema::ErrorType::UnknownError);
}

Schema::ValidationResult Schema::ValidateCompare(SchemaValue schemaValue, YamlValue yamlValue, ValidationResult mismatchError) {
    if (std::holds_alternative<std::shared_ptr<ObjectImplementation>>(schemaValue)) {
        //recursion! (object)

        //yamlValue also needs to be of object type or the types don't match
        if (!std::holds_alternative<std::shared_ptr<parser_types::Object>>(yamlValue))
            return mismatchError;// GetValidationErrorUnexpected(SchemaError::ObjectError(yamlObject, yamlObjectKey), yamlValue);
        //return GetValidationError(SchemaError::ObjectError(yamlObject, yamlObjectKey), Schema::ErrorType::UnexpectedValue);  //ValidationUnexpectedValue;

        ValidationResult result = Schema::Validate(std::get<std::shared_ptr<parser_types::Object>>(yamlValue), std::get<std::shared_ptr<ObjectImplementation>>(schemaValue));

        //if we get an error return, otherwise the object is good and we can check the other keys
        if (std::holds_alternative<ValidationResult::ValidationError>(result.result))
            return result;
    }

    else if (std::holds_alternative<std::shared_ptr<ArrayImplementation>>(schemaValue)) {
        //recursion! (array)

        if (!std::holds_alternative<std::shared_ptr<parser_types::Array>>(yamlValue))
            return mismatchError;// GetValidationErrorUnexpected(SchemaError::ObjectError(yamlObject, yamlObjectKey), yamlValue);

        ValidationResult result = Schema::Validate(std::get<std::shared_ptr<parser_types::Array>>(yamlValue), std::get<std::shared_ptr<ArrayImplementation>>(schemaValue));

        if (std::holds_alternative<ValidationResult::ValidationError>(result.result))
            return result;
    }
    else if (std::holds_alternative<Either>(schemaValue)) {
        //the object value does not have the right type

        Either schemaObjectEither = std::get<Either>(schemaValue);

        if (!Schema::compareTypeToParserType(schemaObjectEither, yamlValue))
            return mismatchError;// GetValidationErrorMismatch(SchemaError::ObjectError(yamlObject, yamlObjectKey), std::get<Either>(schemaValue), yamlValue);

        bool ok = false;

        for (SchemaValue& schemaObjectEitherValue : schemaObjectEither.values) {

            if (!Schema::compareTypeToParserType(schemaObjectEitherValue, yamlValue))
                continue;

            if (std::holds_alternative<std::shared_ptr<ObjectImplementation>>(schemaObjectEitherValue)) {
                //because of the previous compareTypeToParserType check we know that yamlValue must be an Object.
                ValidationResult result = Schema::Validate(std::get<std::shared_ptr<parser_types::Object>>(yamlValue), std::get<std::shared_ptr<ObjectImplementation>>(schemaObjectEitherValue));

                //if we get an error return, otherwise the object is good and we can check the other keys
                if (std::holds_alternative<ValidationResult::ValidationSuccess>(result.result)) {
                    ok = true;
                    break;
                }
            }
            else if (std::holds_alternative<std::shared_ptr<ArrayImplementation>>(schemaObjectEitherValue)) {
                //because of the previous compareTypeToParserType check we know that yamlValue must be an Object.
                ValidationResult result = Schema::Validate(std::get<std::shared_ptr<parser_types::Array>>(yamlValue), std::get<std::shared_ptr<ArrayImplementation>>(schemaObjectEitherValue));

                //if we get an error return, otherwise the object is good and we can check the other keys
                if (std::holds_alternative<ValidationResult::ValidationSuccess>(result.result)) {
                    ok = true;
                    break;
                }
            }
            else {
                ok = true;
                break;
            }
        }

        if (!ok)
            return mismatchError;// GetValidationErrorMismatch(SchemaError::ObjectError(yamlObject, yamlObjectKey), std::get<Either>(schemaValue), yamlValue);
    }  //the object value does not have the right type
    else if (!Schema::compareTypeToParserType(std::get<Type>(schemaValue), yamlValue))
        return mismatchError;// GetValidationErrorMismatch(SchemaError::ObjectError(yamlObject, yamlObjectKey), std::get<Type>(schemaValue), yamlValue);
    
    return GetValidationSuccess;
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

            ValidationResult mismatchError = GetValidationErrorMismatch(SchemaError::ObjectError(yamlObject, yamlObjectKey), schemaObjectValue, yamlObjectValue);

            ValidationResult result = ValidateCompare(schemaObjectValue, yamlObjectValue, mismatchError);

            if (std::holds_alternative<ValidationResult::ValidationError>(result.result))
                return result;
        }


        return GetValidationSuccess;
    }

    // 'yaml' MUST be of type Array

    if (!std::holds_alternative<std::shared_ptr<ArrayImplementation>>(schema))
        return GetValidationError(std::nullopt, Schema::ErrorType::UnexpectedValue);

    std::shared_ptr<ArrayImplementation> schemaArray = std::get<std::shared_ptr<ArrayImplementation>>(schema);
    SchemaValue schemaArrayType = schemaArray->type; //the type that each element of the array needs to be. (can't be object or array)

    std::shared_ptr<parser_types::Array> yamlArray = std::get<std::shared_ptr<parser_types::Array>>(yaml);

    for (int i = 0; i < yamlArray->Size(); i++) {
        std::optional<parser_types::YamlValue> yamlArrayValueOptional = yamlArray->Get(i);

        if (yamlArrayValueOptional.has_value()) {
            parser_types::YamlValue yamlArrayValue = yamlArrayValueOptional.value();

            //array and object are not allowed for now.

            ValidationResult mismatchError = GetValidationErrorMismatch(SchemaError::ArrayError(yamlArray, i), schemaArrayType, yamlArrayValue);

            ValidationResult result = ValidateCompare(schemaArrayType, yamlArrayValue, mismatchError);

            if (std::holds_alternative<ValidationResult::ValidationError>(result.result))
                return result;

        }
        else//should never happen but still needs to be handled
            return GetValidationError(std::nullopt, Schema::ErrorType::UnknownError);
    }

    return GetValidationSuccess;
}