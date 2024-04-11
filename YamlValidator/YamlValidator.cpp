#include "Schema.h"

using SchemaTypes = Schema::Types;

int main() {
	Schema blogSchema({
		{ "title", SchemaTypes::String },
		{ "author", SchemaTypes::String },
		{ "body", SchemaTypes::String },
		{ "body", SchemaTypes::Boolean },
		{ "comments", std::make_shared<Schema::Array>(SchemaTypes::String) },
		{ "meta", std::make_shared<Schema::Object>(std::map<std::string, Schema::SchemaValue>{
			{ "met", SchemaTypes::String }
		})},
	});

	/*
	Schema anotherBlogSchema({
		{ "title", String },
		{ "author", String },
		{ "body", String },
		{ "comments", Array(String) },
		{ "otherComments", Array(Object({
			{ "name", String },
			{ "body", String }
		})) },
		{ "hidden", Boolean },
		{ "hidden", Object({ }) },
		{ "meta", Object({
			1. { "votes", Either(Number, String) },
			2.
			{ "votes", Number }
			{ "votes", Object({
				
			}) }
			{ "favs", Number },
		})}
	});*/
}
