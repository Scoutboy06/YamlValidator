#pragma once

#include <string>
#include <variant>
#include <unordered_map>
#include <vector>
#include <optional>
#include <memory>

namespace parser_types {

	struct String {
		std::string value;
		String(const std::string& value) : value(value) {}
		String(const char value[]) : value(value) {}

		bool operator<(const String& other) const { return value < other.value; };
	};

	struct Number {
		std::string value;
		Number(const std::string& value) : value(value) {}
		Number(const char value[]) : value(value) {}
	};

	struct Boolean {
		bool value;
		Boolean(const bool value) : value(value) {}
	};

	struct Null {};

	class Object;
	class Array;

	using YamlValue = std::variant<String, Number, Boolean, Null, std::shared_ptr<Object>, std::shared_ptr<Array>>;
	using Yaml = std::variant<std::shared_ptr<Object>, std::shared_ptr<Array>>;

	class Object {
	private:
		std::map<std::string, YamlValue> map;

	public:
		Object() {}

		void Set(const std::string& key, YamlValue& value) {
			map.emplace(key, std::move(value));
		}

		void Set(std::pair<std::string, YamlValue> kv) {
			map.emplace(kv.first, std::move(kv.second));
		}

		std::optional<YamlValue> Get(const std::string& key) const {
			auto it = map.find(key);
			if (it != map.end()) {
				return it->second;
			}
			return std::nullopt;
		}

		std::vector<std::string> ExtractKeys() const {
			std::vector<std::string> keys;
			keys.reserve(map.size());
			for (const auto& pair : map) {
				keys.push_back(pair.first);
			}
			return keys;
		}

		bool ContainsKey(const std::string& key) const {
			return map.find(key) != map.end();
		}

		size_t Size() const {
			return map.size();
		}

		void Clear() {
			map.clear();
		}
	};

	class Array {
	private:
		std::vector<YamlValue> values;

	public:
		Array() {}

		void PushBack(const YamlValue& value) {
			values.push_back(value);
		}

		void PopBack() {
			values.pop_back();
		}

		std::optional<YamlValue> Get(const size_t index) const {
			if (index < values.size()) {
				return std::optional<YamlValue>(values[index]);
			}
			return std::nullopt;
		}

		YamlValue operator[](const size_t index) const {
			return values[index];
		}

		size_t Size() const {
			return values.size();
		}

		bool IsEmpty() const {
			return values.empty();
		}

		void Clear() {
			values.clear();
		}
	};
};