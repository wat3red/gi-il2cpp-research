#include "esp.h"
#include "../sdk/types.h"
#include "../sdk/functions/resolve_funcs.h"
#include "../logger.h"
#include "../config/imgui_config.h"
#include "../config/config.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace features {
	void ESP::DrawUI() {
		ImGuiEx::Checkbox("Enable ESP", config.esp.enabled);
		if (config.esp.enabled) {
			ImGui::Indent();
			ImGuiEx::Checkbox("2D Box", config.esp.box2D);
			ImGuiEx::ColorEdit4(("2D Box color"), config.esp.box2DColor);

			ImGuiEx::Checkbox("3D Box", config.esp.box3D);
			ImGuiEx::ColorEdit4(("3D Box color"), config.esp.box3DColor);

			ImGuiEx::Checkbox("Name", config.esp.name);
			ImGuiEx::ColorEdit4(("Name color"), config.esp.nameColor);
			if (config.esp.name) {
				ImGui::Indent();
				ImGuiEx::SliderFloat(("Name size"), config.esp.nameSize, 8.0f, 24.0f);
				ImGui::Unindent();
			}

			ImGuiEx::Checkbox("Line", config.esp.line);

			ImGuiEx::Checkbox(("Line"), config.esp.line);
			ImGuiEx::ColorEdit4(("Line color"), config.esp.lineColor);
			if (config.esp.line) {
				ImGui::Indent();

				const char* bases[] = { ("Bottom"), ("Center"), ("Top") };
				ImGuiEx::Combo(("Line base"), config.esp.lineBase, bases, IM_ARRAYSIZE(bases));
				const char* targets[] = { ("Bottom"), ("Center"), ("Top") };
				ImGuiEx::Combo(("Line target"), config.esp.lineTarget, targets, IM_ARRAYSIZE(targets));

				ImGuiEx::SliderFloat(("Line thickness"), config.esp.lineThickness, 1.0f, 5.0f);
				ImGui::Unindent();
			}
			
			ImGui::Unindent();
		}
	}

	static void DrawSvaston(const ImRect& entityRect, const ImColor& color)
	{
		if (entityRect.Min.x == 0 && entityRect.Min.y == 0 && entityRect.Max.x == 0 && entityRect.Max.y == 0)
			return;

		auto draw = ImGui::GetBackgroundDrawList();

		float xMid = (entityRect.Min.x + entityRect.Max.x) * 0.5f;
		float yMid = (entityRect.Min.y + entityRect.Max.y) * 0.5f;

		draw->AddLine({ xMid, entityRect.Min.y }, { xMid, entityRect.Max.y }, color, 2.0f);
		draw->AddLine({ entityRect.Min.x, yMid }, { entityRect.Max.x,  yMid }, color, 2.0f);

		draw->AddLine({ entityRect.Min }, { entityRect.Min.x,  yMid }, color, 2.0f);
		draw->AddLine({ xMid, entityRect.Min.y }, { entityRect.Max.x, entityRect.Min.y }, color, 2.0f);
		draw->AddLine({ entityRect.Max.x, yMid }, { entityRect.Max.x, entityRect.Max.y }, color, 2.0f);
		draw->AddLine({ xMid, entityRect.Max.y }, { entityRect.Min.x, entityRect.Max.y }, color, 2.0f);
	}

	struct ESPItem {
		MoleMole::BaseEntity* entity;
		ImRect rect;
	};

	static std::vector<ESPItem> esp_items;

	void ESP::DrawBackgroundUI() {
		auto* draw = ImGui::GetBackgroundDrawList();
		if (!draw) return;

		for (const auto& item : esp_items) {
			DrawSvaston(item.rect, ImColor(255, 255, 0, 255));
		}
	}

	void ESP::OnInit() {
	}

	void ESP::OnUpdate() {
		esp_items.clear();

		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::GetEntityManager();
		if (!entity_manager) return;

		Unity::Camera* camera = Unity::Camera::GetMain();
		if (!camera) return;

		auto viewport = ImGui::GetMainViewport();

		std::vector<MoleMole::BaseEntity*> entities = entity_manager->GetEntities();
		for (auto* entity : entities) {
			if (!entity) continue;

			MoleMole::EntityType type = entity->GetType();
			if (type != MoleMole::EntityType::NPC && type != MoleMole::EntityType::Monster)
				continue;

			Unity::GameObject* go = entity->GetGameObject();
			if (!go) continue;

			Unity::Bounds bounds = StageManager_GetBounds(go);  // your requirement
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

			float minX = 99999, minY = 99999;
			float maxX = -99999, maxY = -99999;

			for (int i = 0; i < 8; i++) {
				Unity::Vector3 p = camera->WorldToViewportPoint(corners[i]);
				if (p.z < 0.01f) goto skipEntity;

				float sx = p.x * viewport->Size.x;
				float sy = (1.0f - p.y) * viewport->Size.y;

				minX = min(minX, sx);
				minY = min(minY, sy);
				maxX = max(maxX, sx);
				maxY = max(maxY, sy);
			}

			esp_items.push_back({
				entity,
				ImRect(ImVec2(minX, minY), ImVec2(maxX, maxY))
				});

		skipEntity:
			continue;
		}
	}
}
