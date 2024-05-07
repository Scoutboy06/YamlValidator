#include "Schema.h"

int main() {
    //Schema blogSchema(Schema::CreateObject( {
    //    {"hello", Schema::String},
    //    {"number", Schema::Number},
    //    {"null", Schema::Null},
    //    {"boolean", Schema::Boolean},
    //    { "star wars", Schema::CreateObject({ 
    //        {"obi-wan", Schema::String},
    //        {"anakin", Schema::String},
    //    })},
    //    {"likes", Schema::Number},
    //}));

    //Schema::ValidationResult result = blogSchema.ValidateFromFile("yaml_object.yaml");

    Schema blogSchema(Schema::CreateArray(Schema::Either(Schema::String, Schema::Boolean, Schema::CreateObject({{ "hello", Schema::String }}),
                                          Schema::Null, Schema::Timestamp, Schema::CreateArray(Schema::Either(Schema::Number, Schema::Boolean)))));

    Schema::ValidationResult result = blogSchema.ValidateFromFile("yaml_array.yaml");
}


//- abc
//- 123
//- false
//- ~
//- -Nested array
//- "Double \n quoted"
//- 'Single \n quoted'
//- 2024 - 02 - 17