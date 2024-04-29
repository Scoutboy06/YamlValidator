#include "YamlParser.h"

int main() {
	auto yaml = ParseYaml("yaml_array.yaml");

	/*Schema blogSchema({
		{ "title", Schema::String() },
		{ "author", Schema::Number() },
		{ "body", Schema::String() }
	});*/

	/*
	Schema anotherBlogSchema({
		{ "title", String },
		{ "author", String },
		{ "body", String },
		{ "comments", Array(
			{ "body", String },
			{ "date", String }
		)},
		{ "hidden", Boolean },
		{ "meta", Object({
			{ "votes", Number },
			{ "favs", Number },
		})}
	});*/
}
