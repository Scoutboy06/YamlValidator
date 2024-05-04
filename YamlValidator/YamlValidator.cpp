#include "Schema.h"

int main() {
	Schema blogSchema(Schema::CreateObject( {
		{"hello", Schema::String},
		{"number", Schema::Number},
		{"null", Schema::Null},
		{"boolean", Schema::Boolean},
		{ "star wars", Schema::CreateObject({ 
			{"obi-wan", Schema::String},
			{"anakin", Schema::String},
		})},
		{"likes", Schema::Number},
	}));

	Schema::ValidationResult result = blogSchema.ValidateFromFile("yaml_object.yaml");
}
