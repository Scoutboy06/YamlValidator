#include "Schema.h"

int main() {
	Schema blogSchema(std::unordered_map<std::string, Schema::SchemaValue>{
		{ "title", Schema::Either(Schema::String)},
		{ "author", Schema::String },
		{ "body", Schema::String },
		{ "body", Schema::Boolean },
		{ "comments", Schema::CreateArray(Schema::String) },
		{ "meta", Schema::CreateObject({
			{ "met", Schema::String },
			{"abc", Schema::CreateObject({
				{"lads", Schema::Null}
			})}
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
