#include "esp.h"

#include <imgui/imgui_internal.h>

#include <string>
#include <mutex>

// Mutex to prevent reading while writing
std::mutex esp_mutex;

const char* GetEntityTypeName(int32_t type) {
	auto entity_type = config.esp.GetEntityType(type);
	if (entity_type) {
		return entity_type->type_name.c_str();
	}
	return "Unknown";
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
		int32_t type;
	};

	// Global list used ONLY for drawing
	static std::vector<ESPItem> esp_draw_list;

	// Debug mode: disables all filters and shows all entities
	static bool esp_debug_enabled = false;

	void ESP::DrawUI() {
		ImGuiEx::Checkbox("Enable ESP", config.esp.enabled);

		if (!config.esp.enabled) return;

		ImGui::Indent();

		ImGuiEx::Checkbox("Show Distance", config.esp.distance);
		ImGuiEx::SliderFloat("Max Distance (m)", config.esp.maxDistance, 1.0f, 500.0f);

		const char* bases[] = { "Bottom", "Center", "Top" };
		ImGuiEx::Combo("Line Origin", config.esp.lineBase, bases, IM_ARRAYSIZE(bases));

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::Checkbox("Debug Mode", &esp_debug_enabled)) {
			// Toggle changed
		}
		if (esp_debug_enabled) {
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Debug: All filters disabled, showing all entities");
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::CollapsingHeader("Entity Types", ImGuiTreeNodeFlags_DefaultOpen)) {
			static int32_t selected_type = 2; // Default to Monster

			// Entity type selector
			if (ImGui::BeginCombo("Entity Type", config.esp.GetEntityType(selected_type) ?
				config.esp.GetEntityType(selected_type)->type_name.c_str() : "Select...")) {
				for (auto& [typeID, entity_cfg] : config.esp.entity_types) {
					bool is_selected = (selected_type == typeID);
					if (ImGui::Selectable(entity_cfg.type_name.c_str(), is_selected)) {
						selected_type = typeID;
					}
					if (is_selected) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			auto* selected_entity = config.esp.GetEntityType(selected_type);
			if (selected_entity) {
				ImGui::Spacing();

				// Enable/Disable toggle
				ImGuiEx::Checkbox("##enable", selected_entity->enabled);
				ImGui::SameLine();
				ImGui::Text("Enabled");

				if (selected_entity->enabled) {
					// Box 2D
					ImGuiEx::Checkbox("2D Box##box2d", selected_entity->box2D);
					ImGui::SameLine();
					ImGuiEx::ColorEdit4("##box2DColor", selected_entity->box2DColor, ImGuiColorEditFlags_NoInputs);

					// Name
					ImGuiEx::Checkbox("Name##name", selected_entity->name);
					ImGui::SameLine();
					ImGuiEx::ColorEdit4("##nameColor", selected_entity->nameColor, ImGuiColorEditFlags_NoInputs);
					if (selected_entity->name) {
						ImGuiEx::SliderFloat("Text Size##nameSize", selected_entity->nameSize, 8.0f, 24.0f);
					}

					// Snaplines
					ImGuiEx::Checkbox("Snaplines##line", selected_entity->line);
					ImGui::SameLine();
					ImGuiEx::ColorEdit4("##lineColor", selected_entity->lineColor, ImGuiColorEditFlags_NoInputs);
					if (selected_entity->line) {
						ImGuiEx::SliderFloat("Thickness##lineThickness", selected_entity->lineThickness, 1.0f, 5.0f);
					}
				}
			}
		}

		ImGui::Unindent();
	}

	static void DrawEntityESP(const ESPItem& item) {
		auto draw = ImGui::GetBackgroundDrawList();

		auto* entity_cfg = config.esp.GetEntityType(item.type);
		if (!entity_cfg || !entity_cfg->enabled) {
			// In debug mode, show all entities regardless of config
			if (!esp_debug_enabled) return;
		}

		// 1. Box
		if (entity_cfg && entity_cfg->box2D) {
			draw->AddRect(item.rect.Min - ImVec2(1, 1), item.rect.Max + ImVec2(1, 1), IM_COL32(0, 0, 0, 255));
			draw->AddRect(item.rect.Min + ImVec2(1, 1), item.rect.Max - ImVec2(1, 1), IM_COL32(0, 0, 0, 255));
			draw->AddRect(item.rect.Min, item.rect.Max, ImGui::ColorConvertFloat4ToU32(entity_cfg->box2DColor.GetValue()));
		} else if (esp_debug_enabled) {
			// In debug mode, draw white boxes for all entities
			draw->AddRect(item.rect.Min - ImVec2(1, 1), item.rect.Max + ImVec2(1, 1), IM_COL32(0, 0, 0, 255));
			draw->AddRect(item.rect.Min + ImVec2(1, 1), item.rect.Max - ImVec2(1, 1), IM_COL32(0, 0, 0, 255));
			draw->AddRect(item.rect.Min, item.rect.Max, IM_COL32(255, 255, 255, 255));
		}

		// 2. Snaplines
		if (entity_cfg && entity_cfg->line) {
			ImVec2 screenSize = ImGui::GetMainViewport()->Size;
			ImVec2 startPos;
			switch (config.esp.lineBase) {
			case 0: startPos = { screenSize.x / 2, screenSize.y }; break;
			case 1: startPos = { screenSize.x / 2, screenSize.y / 2 }; break;
			case 2: startPos = { screenSize.x / 2, 0 }; break;
			}
			draw->AddLine(startPos, item.bottom, ImGui::ColorConvertFloat4ToU32(entity_cfg->lineColor.GetValue()), entity_cfg->lineThickness);
		}

		// 3. Text
		if ((entity_cfg && (entity_cfg->name || config.esp.distance)) || esp_debug_enabled) {
			std::string label;
			if (esp_debug_enabled) {
				// In debug mode: show name + type + distance
				label = item.name + " [Type: " + GetEntityTypeName(item.type) + "]";
				label += " [" + std::to_string((int)item.distance) + "m]";
			} else {
				if (entity_cfg->name) label += item.name;
				if (entity_cfg->name && config.esp.distance) label += " ";
				if (config.esp.distance) label += "[" + std::to_string((int)item.distance) + "m]";
			}

			if (!label.empty()) {
				float fontSize = (entity_cfg) ? entity_cfg->nameSize : 12.0f;
				ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, label.c_str());
				ImVec2 textPos = { item.top.x - (textSize.x / 2), item.top.y - textSize.y - 2.0f };

				draw->AddText(ImGui::GetFont(), fontSize, textPos + ImVec2(1, 1), IM_COL32(0, 0, 0, 255), label.c_str());
				uint32_t textColor = (entity_cfg) ? ImGui::ColorConvertFloat4ToU32(entity_cfg->nameColor.GetValue()) : IM_COL32(255, 255, 0, 255);
				if (esp_debug_enabled) textColor = IM_COL32(0, 255, 0, 255); // Green for debug mode
				draw->AddText(ImGui::GetFont(), fontSize, textPos, textColor, label.c_str());
			}
		}
	}

	void ESP::DrawBackgroundUI() {
		if (!config.esp.enabled) return;

		// LOCK: We are reading the list now
		std::lock_guard<std::mutex> lock(esp_mutex);

		for (const auto& item : esp_draw_list) {
			DrawEntityESP(item);
		}
	}

	void ESP::OnUpdate() {
		if (!config.esp.enabled) return;

		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		if (!entity_manager) return;

		Unity::Camera* camera = Unity::Camera::GetMain();
		if (!camera) return;

		Unity::Transform* cameraTransform = camera->GetTransform();
		if (!cameraTransform) return;

		auto viewport = ImGui::GetMainViewport();

		std::vector<ESPItem> temp_list;
		std::vector<MoleMole::BaseEntity*> entities = entity_manager->GetEntities();

		for (auto* entity : entities) {
			if (!entity) continue;

			int32_t type = (int32_t)entity->GetType();

			// Check if this entity type is enabled (unless in debug mode)
			if (!esp_debug_enabled) {
				auto* entity_cfg = config.esp.GetEntityType(type);
				if (!entity_cfg || !entity_cfg->enabled) {
					continue;
				}
			}

			Unity::GameObject* go = entity->GetGameObject();
			if (!go) continue;

			float dist = entity_manager->GetAvatar()->GetAbsolutePosition().Distance(entity->GetAbsolutePosition());

			// Skip distance filter in debug mode
			if (!esp_debug_enabled && dist > config.esp.maxDistance) continue;

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
			item.type = type;
			item.rect = ImRect(ImVec2(minX, minY), ImVec2(maxX, maxY));
			item.distance = dist;
			item.top = ImVec2((minX + maxX) * 0.5f, minY);
			item.bottom = ImVec2((minX + maxX) * 0.5f, maxY);

			auto* namePtr = entity->GetName();
			if (namePtr) item.name = namePtr->ToCStr();
			if (item.name.empty()) item.name = GetEntityTypeName(type);

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