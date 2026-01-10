#define IMGUI_DEFINE_MATH_OPERATORS

#include "map_teleport.h"

#include <game_api/include.h>
#include <logger/logger.h>
#include <config/imgui_config.h>
#include <config/config.h>

#include <imgui/imgui.h>
#include <minhook/include/MinHook.h>
#include <imgui/imgui_internal.h>

namespace features {
	void MapTeleport::DrawUI() {
		ImGuiEx::Checkbox("Enable map teleport", config.map_teleport.enabled);
	}

	void MapTeleport::DrawBackgroundUI() {}

	void OnMapClicked_Internal(void* _this, Unity::Vector2 screenPos) {
		Il2CppObject* ui_manager = MoleMole::SingletonManager::GetSingletonInstance("BHDAEIAPFDE");
		if (!ui_manager) return;

		Unity::Camera* ui_camera = *reinterpret_cast<Unity::Camera**>((uintptr_t)ui_manager + 0x2A8);
		if (!ui_camera) return;

		void* mono_in_level_map_page = *reinterpret_cast<void**>((uintptr_t)_this + 0x380);
		if (!mono_in_level_map_page) return;

		auto map_background = Il2Cpp::Method::Call<Unity::Transform*>("MoleMole", "MonoInLevelMapPage", "get_mapBackground", 0, mono_in_level_map_page);
		if (!map_background) return;

		Unity::Vector2 levelPos = { 0,0 };
		if (Il2Cpp::Method::Call<bool>("UnityEngine", "RectTransformUtility", "ScreenPointToLocalPointInRectangle", 4, map_background, screenPos, ui_camera, levelPos))
		{
			auto mapRect = Il2Cpp::Method::Call<Unity::Rect>("MoleMole", "MonoInLevelMapPage", "get_mapRect", 0, mono_in_level_map_page);
			Unity::Rect mapViewRect = *reinterpret_cast<Unity::Rect*>((uintptr_t)_this + 0x570); // 0x570, 0x5E8

			levelPos.x = (levelPos.x - mapRect.m_XMin) / mapRect.m_Width;
			levelPos.x = (levelPos.x * mapViewRect.m_Width) + mapViewRect.m_XMin;

			levelPos.y = (levelPos.y - mapRect.m_YMin) / mapRect.m_Height;
			levelPos.y = (levelPos.y * mapViewRect.m_Height) + mapViewRect.m_YMin;

			auto worldPos = Il2Cpp::Method::Call<Unity::Vector3>("MoleMole", "Miscs", "GenWorldPos", 1, levelPos);
			auto relativePos = Il2Cpp::Method::Call<Unity::Vector3>("MoleMole", "WorldShiftManager", "GetRelativePosition", 1, worldPos);
			worldPos.y = Il2Cpp::Method::Call<float>("MoleMole", "Miscs", "CalcCurrentGroundHeight", 2, relativePos.x, relativePos.z) + 10.f;

			Log("worldPos: %n\n", worldPos.ToString());

			//std::thread(TeleportBase::TeleportTo, worldPos, 0).detach();

			BasePageContext_ClosePage(_this);
		}
	}

	// to find "class MoleMole.BasePageContext " just search for "public virtual Void ClosePage();"
	// "class MoleMole.UIManager " found via "public UIPlatformConfig " BHDAEIAPFDE
	// "class MoleMole.InLevelMapPageContext " found via "private MonoInLevelMapPage " ABFGCHAHIIO
	void (*InLevelMapPageContext_OnMapClicked)(void* _this, Unity::Vector2 screenPos);
	void hInLevelMapPageContext_OnMapClicked(void* _this, Unity::Vector2 screenPos) {
		OnMapClicked_Internal(_this, screenPos);

		InLevelMapPageContext_OnMapClicked(_this, screenPos);
	}

	void MapTeleport::OnInit() {
		auto sig = Mem::Signature("41 57 41 56 56 57 53 48 81 EC ? ? ? ? 44 0F 29 44 24 ? 0F 29 7C 24 ? 0F 29 74 24 ? 49 89 D6"); // sub_D1FB080
		MH_CreateHook((LPVOID)(sig.Scan()), (LPVOID)hInLevelMapPageContext_OnMapClicked, (LPVOID*)&InLevelMapPageContext_OnMapClicked); // 0xD1FB080, 0xD1F52E0, 0xD1974D0, 0xD198150
	}

	void MapTeleport::OnUpdate() {}
}