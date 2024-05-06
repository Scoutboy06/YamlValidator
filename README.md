# YamlValidator

YamlValidator is a robust and efficient C++ library designed to validate YAML files. It leverages the power of modern C++ features to provide a seamless and user-friendly interface for YAML validation. The library is built with a focus on performance, ease of use, and extensibility.

The YamlValidator library parses YAML files and validates them against a predefined schema. It uses a combination of two main components: YamlParser and Schema. The YamlParser is responsible for parsing the YAML file into a structured format that can be easily manipulated. The Schema component, on the other hand, defines the structure that the YAML file should adhere to and validates the parsed YAML against this schema.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Cloning the Repository

To clone the repository, follow these steps:

1. Open your terminal.
2. Navigate to the directory where you want to clone the repository.
3. Run the following command:

```bash
git clone https://github.com/Scoutboy06/YamlValidator.git
```

### Contributing

If you want to contribute to this project, please follow these steps:

1. Fork the repository.
2. Create a new branch in your forked repository.
3. Make your changes in the new branch.
4. Commit your changes and push the branch to your forked repository.
5. Open a pull request from the new branch in your forked repository to the master branch in the original repository.

## Usage

Here is an example of how you can use YamlValidator to validate a YAML file:

### YAML File (yaml_object.yaml)

```yaml
string: textgoeshere
number: 123
null: ~
boolean: false
object:
  string: text
```

### Code (YamlValidator.cpp)

```cpp
#include "Schema.h"

int main() {
    // Define the schema for the YAML file
    Schema blogSchema(Schema::CreateObject( {
		{"string", Schema::String},
		{"number", Schema::Number},
		{"null", Schema::Null},
		{"boolean", Schema::Boolean},
		{ "object", Schema::CreateObject({
			{"string", Schema::String}
		})}
	}));

    // Validate the YAML file against the schema
    Schema::ValidationResult result = blogSchema.ValidateFromFile("yaml_object.yaml");
}
```

In the above code, we first define a schema for the YAML file. The schema specifies the expected structure of the YAML file. Then, we validate the YAML file against the schema using the `ValidateFromFile` method of the `Schema` class. The `ValidateFromFile` method returns a `ValidationResult` object that contains the result of the validation.

## License

This project is licensed under the WTFPL License. See the [LICENSE](LICENSE) file for details.
