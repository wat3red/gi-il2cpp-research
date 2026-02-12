#define IMGUI_DEFINE_MATH_OPERATORS

#include "map_teleport.h"

#include <game_api/include.h>
#include <logger/logger.h>
#include <config/imgui_config.h>
#include <config/config.h>

#include <thread>

#include <imgui/imgui.h>
#include <minhook/include/MinHook.h>
#include <imgui/imgui_internal.h>
#include <game_api/version_constants.h>

namespace features
{
	void MapTeleport::DrawUI() {
		ImGuiEx::Checkbox("Enable map teleport", config.map_teleport.enabled);
	}

	// to find "class MoleMole.BasePageContext " just search for "public virtual Void ClosePage();"
	// "class MoleMole.UIManager " found via "public UIPlatformConfig "
	// "class MoleMole.InLevelMapPageContext " found via "private MonoInLevelMapPage "
	void OnMapClicked_Internal(MoleMole::InLevelMapPageContext* _this, Unity::Vector2 screenPos) {
		// "private Dictionary<Int32,List<Notify>> " or "private MonoBaseCanvas "
		MoleMole::UIManager* ui_manager = MoleMole::UIManager::Instance();
		if (!ui_manager) return;

		Unity::Camera* ui_camera = ui_manager->GetUICamera();
		if (!ui_camera) return;

		MoleMole::MonoInLevelMapPage* page_mono = _this->GetPageMono();
		if (!page_mono) return;

		Unity::Transform* map_background = page_mono->GetMapBackground();
		if (!map_background) return;

		Unity::Vector2 levelPos = { 0,0 };
		if (Il2Cpp::Method::Call<bool>("UnityEngine", "RectTransformUtility", "ScreenPointToLocalPointInRectangle", 4, map_background, screenPos, ui_camera, &levelPos)) {
			Unity::Rect mapRect = page_mono->GetMapRect();
			Unity::Rect mapViewRect = _this->GetMapViewRect();

			levelPos.x = (levelPos.x - mapRect.m_XMin) / mapRect.m_Width;
			levelPos.x = (levelPos.x * mapViewRect.m_Width) + mapViewRect.m_XMin;

			levelPos.y = (levelPos.y - mapRect.m_YMin) / mapRect.m_Height;
			levelPos.y = (levelPos.y * mapViewRect.m_Height) + mapViewRect.m_YMin;

			auto worldPos = Il2Cpp::Method::Call<Unity::Vector3>("MoleMole", "Miscs", "GenWorldPos", 1, levelPos);
			auto relativePos = Il2Cpp::Method::Call<Unity::Vector3>("MoleMole", "WorldShiftManager", "GetRelativePosition", 1, worldPos);
			worldPos.y = Il2Cpp::Method::Call<float>("MoleMole", "Miscs", "CalcCurrentGroundHeight", 2, relativePos.x, relativePos.z) + 5.f;

			MoleMole::ActorUtils::SetAvatarPos(worldPos);
			MoleMole::EntityManager* entityManager = MoleMole::EntityManager::Instance();
			MoleMole::ActorUtils::SyncEntityPos(entityManager->GetAvatar(), 0, 0);
			
			BasePageContext_ClosePage(_this);
			NullReferenceException();
		}
	}

	void (*InLevelMapPageContext_OnMapClicked)(MoleMole::InLevelMapPageContext* _this, Unity::Vector2 screenPos);
	void hInLevelMapPageContext_OnMapClicked(MoleMole::InLevelMapPageContext* _this, Unity::Vector2 screenPos) {
		if (config.map_teleport.enabled) {
			OnMapClicked_Internal(_this, screenPos);
		}

		InLevelMapPageContext_OnMapClicked(_this, screenPos);
	}

	void MapTeleport::OnInit() {
		MH_CreateHook((LPVOID)(Mem::Signature("41 57 41 56 56 57 53 48 81 EC ? ? ? ? 44 0F 29 44 24 ? 0F 29 7C 24 ? 0F 29 74 24 ? 49 89 D6").Scan()),
			(LPVOID)hInLevelMapPageContext_OnMapClicked, (LPVOID*)&InLevelMapPageContext_OnMapClicked);

		//todo: hook InLevelMapPageContext_OnMarkClicked
		//private void [A-Z]{11}\(MonoMapMark IDLDOMJBBEK)
	}

}