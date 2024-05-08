#include "Schema.h"

int main() {
    Schema blogSchema(Schema::CreateObject( {
        {"media", Schema::String},
        {"content", Schema::CreateArray( 
            Schema::CreateObject({
                { "name", Schema::String },
                { "label", Schema::String },
                { "type", Schema::String },
                { "path", Schema::String },
                { "fields", Schema::CreateArray(
                    Schema::CreateObject({
                        { "name", Schema::String },
                        { "label", Schema::String },
                        { "type", Schema::String },
                        { "hidden", Schema::Boolean },
                        { "default", Schema::String },
                        { "options", Schema::CreateObject({ {"maxlength", Schema::Boolean} }) },
                    })
                )}
            })
        )}
    }));

    Schema::ValidationResult result = blogSchema.ValidateFromFile("examples/yaml_example.yaml");

    //Schema blogSchema(Schema::CreateArray(Schema::Either(Schema::String, Schema::Boolean, Schema::CreateObject({{ "hello", Schema::String }}),
    //                                      Schema::Null, Schema::Timestamp, Schema::CreateArray(Schema::Either(Schema::Number, Schema::Boolean)))));

    //Schema::ValidationResult result = blogSchema.ValidateFromFile("yaml_array.yaml");
}