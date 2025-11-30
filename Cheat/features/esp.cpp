#include "esp.h"
#include "../sdk/types.h"
#include "../sdk/functions/resolve_funcs.h"
#include "../logger.h"

#include <imgui/imgui.h>

namespace features {
	void ESP::DrawUI() {
		//ImGui::Checkbox("Enable ESP", &enabled);
	}

	std::vector<Unity::Vector3> screen_positions;

	void ESP::DrawBackgroundUI() {
		auto* draw = ImGui::GetBackgroundDrawList();
		if (!draw) return;
		auto& io = ImGui::GetIO();
		ImVec2 center = { io.DisplaySize.x / 2, io.DisplaySize.y / 2 };

		for (const auto& screen : screen_positions) {
			draw->AddLine(
				center,
				ImVec2(screen.x, screen.y),
				IM_COL32(255, 255, 255, 255),
				2.0f
			);
		}
	}

	void ESP::OnInit() {
	}

	// Helper function to get camera's pixel dimensions
	Unity::Vector2 GetCameraPixelSize(Unity::Camera* camera) {
		Unity::Vector2 size;
		size.x = (float)Camera_get_pixelWidth(camera);
		size.y = (float)Camera_get_pixelHeight(camera);
		return size;
	}

	void ESP::OnUpdate() {
		screen_positions.clear();

		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::GetEntityManager();
		if (!entity_manager) return;

		std::vector<MoleMole::BaseEntity*> entities = entity_manager->GetEntities();
		auto viewport = ImGui::GetMainViewport();

		for (MoleMole::BaseEntity* entity : entities) {
			if (!entity) continue;

			Unity::GameObject* game_object = entity->GetGameObject();
			if (!game_object) continue;

			Unity::Transform* transform = game_object->GetTransform();
			if (!transform) continue;
			Unity::Vector3 pos = transform->GetPosition();

			Unity::Camera* camera = Unity::Camera::GetMain();
			if (!camera) continue;

			// METHOD 1: Using WorldToScreenPoint (corrected)
			Unity::Vector3 screen_pos = camera->WorldToScreenPoint(pos);

			if (screen_pos.z < 0.01f) continue;

			// The key fix: Unity's screen coordinates might need different handling
			Unity::Vector3 final_pos;

			// Option B: Scale to current viewport (most likely needed)
			// Get the camera's pixel dimensions to understand the coordinate system
			Unity::Vector2 camera_pixel_size = GetCameraPixelSize(camera);
			if (camera_pixel_size.x > 0 && camera_pixel_size.y > 0) {
				final_pos.x = (screen_pos.x / camera_pixel_size.x) * viewport->Size.x;
				final_pos.y = viewport->Size.y - (screen_pos.y / camera_pixel_size.y) * viewport->Size.y;
			}
			else {
				// Fallback: Assume coordinates are already in screen space but might need clamping
				final_pos.x = screen_pos.x;
				final_pos.y = viewport->Size.y - screen_pos.y;

				// Clamp to screen bounds
				if (final_pos.x < 0 || final_pos.x > viewport->Size.x ||
					final_pos.y < 0 || final_pos.y > viewport->Size.y) {
					continue; // Skip if out of bounds
				}
			}

			screen_positions.push_back(final_pos);
		}
	}
}
