#include "noclip.h"

namespace features
{
	void Noclip::DrawUI() {
		ImGuiEx::Checkbox("Enable noclip", config.noclip.enabled);
		if (config.noclip.enabled) {
			ImGui::Indent();
			ImGuiEx::Hotkey("Hotkey", config.noclip.enable_hotkey);
			ImGuiEx::SliderFloat("Speed##noclip", config.noclip.speed, 1.f, 20.f);
			ImGui::Unindent();
		}
	}

	void Noclip::OnUpdate() {
		Unity::Camera* main_camera = Unity::Camera::GetMain();
		if (!main_camera) return;

		Unity::Transform* main_camera_trans = main_camera->GetTransform();
		if (!main_camera_trans) return;

		Unity::Vector3 dir = { 0,0,0 };

		if (ImGui::IsKeyDown(ImGuiKey_W))
			dir += main_camera_trans->GetForward();
		if (ImGui::IsKeyDown(ImGuiKey_S))
			dir -= main_camera_trans->GetForward();
		if (ImGui::IsKeyDown(ImGuiKey_A))
			dir -= main_camera_trans->GetRight();
		if (ImGui::IsKeyDown(ImGuiKey_D))
			dir += main_camera_trans->GetRight();
		if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
			dir.y -= 1;
		if (ImGui::IsKeyDown(ImGuiKey_Space))
			dir.y += 1;

		float delta = Unity::Time::GetDeltaTime();

		dir = dir * config.noclip.speed.GetValue() * delta;

		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		if (!entity_manager) return;

		MoleMole::AvatarEntity* avatar = entity_manager->GetAvatar();
		if (!avatar) return;

		Unity::Rigidbody* rigidbody = avatar->GetRigidbody();
		if (!rigidbody) return;

		Unity::Vector3 newPos = avatar->GetAbsolutePosition() + dir;

		if (config.noclip.enabled) {
			rigidbody->SetVelocity({ 0, 0, 0 });
			avatar->SetAbsolutePosition(newPos);
		}

		rigidbody->SetUseGravity(!config.noclip.enabled);
		rigidbody->SetIsKinematic(config.noclip.enabled);
	}

	void Noclip::UpdateHotkeys() {
		if (ImGui::IsKeyDown(config.noclip.enable_hotkey))
			config.noclip.enabled = !config.noclip.enabled;
	}
}