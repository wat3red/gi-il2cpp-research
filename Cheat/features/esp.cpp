#define IMGUI_DEFINE_MATH_OPERATORS

#include "esp.h"

#include <game_api/include.h>
#include <logger/logger.h>
#include <config/imgui_config.h>
#include <config/config.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <string>
#include <mutex> // Required for thread safety

// Mutex to prevent reading while writing
std::mutex esp_mutex;

const char* GetEntityTypeName(int32_t type) {
	switch (type) {
	case 1: return "Avatar";
	case 2: return "Monster";
	case 12: return "NPC";
	case 21: return "GatherPoint";
	case 26: return "Chest";
	case 66: return "BlackMud";
	default: return "Unknown";
	}
}

namespace features
{

	struct ESPItem {
		MoleMole::BaseEntity* entity;
		ImRect rect;
		float distance;
		std::string name;
		ImVec2 top;
		ImVec2 bottom;
	};

	// Global list used ONLY for drawing
	static std::vector<ESPItem> esp_draw_list;

	void ESP::DrawUI() {
		ImGuiEx::Checkbox("Enable ESP", config.esp.enabled);

		if (!config.esp.enabled) return;

		if (ImGui::BeginTabBar("ESP_Tabs")) {
			if (ImGui::BeginTabItem("Visuals")) {
				ImGui::Indent();
				ImGuiEx::Checkbox("2D Box", config.esp.box2D);
				ImGui::SameLine();
				ImGuiEx::ColorEdit4("##Box2DColor", config.esp.box2DColor, ImGuiColorEditFlags_NoInputs);

				ImGuiEx::Checkbox("Name", config.esp.name);
				ImGui::SameLine();
				ImGuiEx::ColorEdit4("##NameColor", config.esp.nameColor, ImGuiColorEditFlags_NoInputs);
				if (config.esp.name) {
					ImGuiEx::SliderFloat("Text Size", config.esp.nameSize, 8.0f, 24.0f);
				}

				ImGuiEx::Checkbox("Show Distance", config.esp.distance);
				ImGuiEx::SliderFloat("Max Distance (m)", config.esp.maxDistance, 1.0f, 500.0f);

				ImGuiEx::Checkbox("Snaplines", config.esp.line);
				ImGui::SameLine();
				ImGuiEx::ColorEdit4("##LineColor", config.esp.lineColor, ImGuiColorEditFlags_NoInputs);
				if (config.esp.line) {
					const char* bases[] = { "Bottom", "Center", "Top" };
					ImGuiEx::Combo("Line Origin", config.esp.lineBase, bases, IM_ARRAYSIZE(bases));
					ImGuiEx::SliderFloat("Thickness", config.esp.lineThickness, 1.0f, 5.0f);
				}
				ImGui::Unindent();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Filters")) {
				ImGui::Text("Select Entities to Draw:");
				ImGui::Separator();
				int columns = 3;
				if (ImGui::BeginTable("FiltersTable", columns)) {
					for (auto& [typeID, configVar] : config.esp.filters) {
						ImGui::TableNextColumn();
						const char* name = GetEntityTypeName(typeID);
						std::string label = std::string(name) + "##" + std::to_string(typeID);
						ImGuiEx::Checkbox(label.c_str(), configVar);
					}
					ImGui::EndTable();
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}

	static void DrawEntityESP(const ESPItem& item, const Config::ESP& cfg) {
		auto draw = ImGui::GetBackgroundDrawList();

		// 1. Box
		if (cfg.box2D) {
			draw->AddRect(item.rect.Min - ImVec2(1, 1), item.rect.Max + ImVec2(1, 1), IM_COL32(0, 0, 0, 255));
			draw->AddRect(item.rect.Min + ImVec2(1, 1), item.rect.Max - ImVec2(1, 1), IM_COL32(0, 0, 0, 255));
			draw->AddRect(item.rect.Min, item.rect.Max, ImGui::ColorConvertFloat4ToU32(cfg.box2DColor.GetValue()));
		}

		// 2. Snaplines
		if (cfg.line) {
			ImVec2 screenSize = ImGui::GetMainViewport()->Size;
			ImVec2 startPos;
			switch (cfg.lineBase) {
			case 0: startPos = { screenSize.x / 2, screenSize.y }; break;
			case 1: startPos = { screenSize.x / 2, screenSize.y / 2 }; break;
			case 2: startPos = { screenSize.x / 2, 0 }; break;
			}
			draw->AddLine(startPos, item.bottom, ImGui::ColorConvertFloat4ToU32(cfg.lineColor.GetValue()), cfg.lineThickness);
		}

		// 3. Text
		if (cfg.name || cfg.distance) {
			std::string label;
			if (cfg.name) label += item.name;
			if (cfg.name && cfg.distance) label += " ";
			if (cfg.distance) label += "[" + std::to_string((int)item.distance) + "m]";

			ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(cfg.nameSize, FLT_MAX, 0.0f, label.c_str());
			ImVec2 textPos = { item.top.x - (textSize.x / 2), item.top.y - textSize.y - 2.0f };

			draw->AddText(ImGui::GetFont(), cfg.nameSize, textPos + ImVec2(1, 1), IM_COL32(0, 0, 0, 255), label.c_str());
			draw->AddText(ImGui::GetFont(), cfg.nameSize, textPos, ImGui::ColorConvertFloat4ToU32(cfg.nameColor.GetValue()), label.c_str());
		}
	}

	void ESP::DrawBackgroundUI() {
		if (!config.esp.enabled) return;

		// LOCK: We are reading the list now
		std::lock_guard<std::mutex> lock(esp_mutex);

		for (const auto& item : esp_draw_list) {
			DrawEntityESP(item, config.esp);
		}
	}

	void ESP::OnInit() {}

	void ESP::OnUpdate() {
		if (!config.esp.enabled) return;

		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		if (!entity_manager) return;

		Unity::Camera* camera = Unity::Camera::GetMain();
		if (!camera) return;

		//Log("%d\n", __LINE__);

		Unity::Transform* cameraTransform = camera->GetTransform();
		if (!cameraTransform) return;

		//Log("%d\n", __LINE__);

		Unity::Vector3 cameraPos = cameraTransform->GetPosition();
		auto viewport = ImGui::GetMainViewport();

		//Log("%d\n", __LINE__);

		// Create a TEMPORARY list. We work on this so we don't disturb the drawing thread.
		std::vector<ESPItem> temp_list;
		std::vector<MoleMole::BaseEntity*> entities = entity_manager->GetEntities();

		//Log("entities.size: %d\n", entities.size());

		for (auto* entity : entities) {
			if (!entity) continue;

			int32_t type = (int32_t)entity->GetType();
			//Log("%d\n", __LINE__);

			// Filter Check (Uncommented and fixed)
			auto it = config.esp.filters.find(type);
			if (it == config.esp.filters.end() || !it->second) {
				continue;
			}
			//Log("%d\n", __LINE__);

			Unity::GameObject* go = entity->GetGameObject();
			if (!go) continue;

			Unity::Transform* transform = go->GetTransform();
			if (!transform) continue;
			//Log("%d\n", __LINE__);

			Unity::Vector3 pos = transform->GetPosition();
			float dist = cameraPos.Distance(pos);

			if (dist > config.esp.maxDistance) continue;
			//Log("%d\n", __LINE__);

			Unity::Bounds bounds = StageManager_GetBounds(go);
			Unity::Vector3 center = bounds.center;
			Unity::Vector3 ext = bounds.extents;

			Unity::Vector3 corners[8] = {
				{center.x - ext.x, center.y - ext.y, center.z - ext.z},
				{center.x + ext.x, center.y - ext.y, center.z - ext.z},
				{center.x - ext.x, center.y + ext.y, center.z - ext.z},
				{center.x + ext.x, center.y + ext.y, center.z - ext.z},
				{center.x - ext.x, center.y - ext.y, center.z + ext.z},
				{center.x + ext.x, center.y - ext.y, center.z + ext.z},
				{center.x - ext.x, center.y + ext.y, center.z + ext.z},
				{center.x + ext.x, center.y + ext.y, center.z + ext.z}
			};

			float minX = FLT_MAX, minY = FLT_MAX;
			float maxX = -FLT_MAX, maxY = -FLT_MAX;
			bool onScreen = false;

			for (int i = 0; i < 8; i++) {
				Unity::Vector3 p = camera->WorldToViewportPoint(corners[i]);

				if (p.z > 0.01f) {
					float sx = p.x * viewport->Size.x;
					float sy = (1.0f - p.y) * viewport->Size.y;

					minX = min(minX, sx);
					minY = min(minY, sy);
					maxX = max(maxX, sx);
					maxY = max(maxY, sy);
					onScreen = true;
				}
			}

			if (!onScreen) continue;

			ESPItem item;
			item.entity = entity;
			item.rect = ImRect(ImVec2(minX, minY), ImVec2(maxX, maxY));
			item.distance = dist;
			item.top = ImVec2((minX + maxX) * 0.5f, minY);
			item.bottom = ImVec2((minX + maxX) * 0.5f, maxY);
			//Log("%d\n", __LINE__);

			// Safe name retrieval
			auto* namePtr = entity->GetName();
			if (namePtr) item.name = namePtr->ToCStr();
			if (item.name.empty()) item.name = GetEntityTypeName(type);
			//Log("%d\n", __LINE__);

			// Push to local list (fast, no locking needed yet)
			temp_list.push_back(item);
		}

		// SWAP: Now we instantly update the draw list
		{
			std::lock_guard<std::mutex> lock(esp_mutex);
			esp_draw_list = std::move(temp_list);
		}
	}
}