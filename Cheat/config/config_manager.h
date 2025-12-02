#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <filesystem>

#include "json.hpp"
#include <imgui/imgui.h> 

namespace nlohmann {
	template <>
	struct adl_serializer<ImColor> {
		static void to_json(json& j, const ImColor& color) {
			j = json{ {("r"), color.Value.x}, {("g"), color.Value.y}, {("b"), color.Value.z}, {("a"), color.Value.w} };
		}

		static void from_json(const json& j, ImColor& color) {
			color = ImColor(j.value(("r"), 1.0f), j.value(("g"), 1.0f), j.value(("b"), 1.0f), j.value(("a"), 1.0f));
		}
	};

	template <>
	struct adl_serializer<ImVec4> {
		static void to_json(json& j, const ImVec4& color) {
			j = json{ {("x"), color.x}, {("y"), color.y}, {("z"), color.z}, {("w"), color.w} };
		}

		static void from_json(const json& j, ImVec4& color) {
			color.x = j.value(("x"), 1.0f);
			color.y = j.value(("y"), 1.0f);
			color.z = j.value(("z"), 1.0f);
			color.w = j.value(("w"), 1.0f);
		}
	};

	template <>
	struct adl_serializer<ImVec3> {
		static void to_json(json& j, const ImVec3& vector) {
			j = json{ {("x"), vector.x}, {("y"), vector.y}, {("z"), vector.z} };
		}

		static void from_json(const json& j, ImVec3& vector) {
			vector.x = j.value(("x"), 1.0f);
			vector.y = j.value(("y"), 1.0f);
			vector.z = j.value(("z"), 1.0f);
		}
	};

	template <>
	struct adl_serializer<ImVec2> {
		static void to_json(json& j, const ImVec2& vector) {
			j = json{ {("x"), vector.x}, {("y"), vector.y} };
		}

		static void from_json(const json& j, ImVec2& vector) {
			vector.x = j.value(("x"), 1.0f);
			vector.y = j.value(("y"), 1.0f);
		}
	};
}

using json = nlohmann::json;

namespace ConfigManager {
	extern std::string CurrentConfig;

	void Save(const json& config);
	json Load();
	void RemoveSection(const std::string& path);
	std::vector<std::string> SplitPath(const std::string& path);

	template<typename T>
	void SetValue(const std::string& path, const std::string& key, const T& value) {
		json config = Load();
		json* section = &config;
		for (const auto& part : SplitPath(path)) {
			section = &((*section)[part]);
		}
		(*section)[key] = value;
		Save(config);
	}

	template<typename T>
	T GetValue(const std::string& path, const std::string& key, const T& defaultValue) {
		json config = Load();
		json* section = &config;
		for (const auto& part : SplitPath(path)) {
			if (!section->contains(part)) return defaultValue;
			section = &((*section)[part]);
		}
		return section->contains(key) ? (*section)[key].get<T>() : defaultValue;
	}
}