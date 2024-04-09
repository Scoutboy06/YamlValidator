#pragma once

#include <string>
#include <variant>
#include <unordered_map>
#include <vector>
#include <optional>

class String {
	const std::string& value;
	String(const std::string& value) : value(value) {}
	String(const char value[]) : value(value) {}

	bool operator<(const String& other) const { return value < other.value; };
};

class Number {
	const std::string& value;
	Number(const std::string& value) : value(value) {}
	Number(const char value[]) : value(value) {}
};

class Boolean {
	bool value;
	Boolean(const bool value) : value(value) {}
};

class Null {};

class Object;
class Array;

using YamlValue = std::variant<String, Number, Boolean, Null, Object, Array>;

class Object {
private:
	std::unordered_map<String, YamlValue> map;

public:
	Object() {}

	void Set(const std::string& key, const YamlValue& value) {
		map.insert_or_assign(key, value);
	}

	void Set(const std::pair<String, YamlValue> kv) {
		map.insert_or_assign(kv.first, kv.second);
	}

	void Set(const std::pair<std::string, YamlValue> kv) {
		map.insert_or_assign(String(kv.first), kv.second);
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
	void PushBack(const YamlValue& value) {
		values.push_back(value);
	}

	void PopBack() {
		values.pop_back();
	}

	std::optional<YamlValue> Get(const size_t index) const {
		if (index < values.size()) {
			return values[index];
		}
		return std::nullopt;
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
}