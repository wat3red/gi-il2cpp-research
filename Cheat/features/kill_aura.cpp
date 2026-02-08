#define IMGUI_DEFINE_MATH_OPERATORS

#include "kill_aura.h"

#include <game_api/include.h>
#include <logger/logger.h>
#include <config/imgui_config.h>
#include <config/config.h>

#include <imgui/imgui.h>
#include <minhook/include/MinHook.h>
#include <imgui/imgui_internal.h>
#include <cfloat> // Required for FLT_MAX

namespace features
{
	void KillAura::DrawUI() {
		ImGuiEx::Checkbox("Enable kill aura", config.kill_aura.enabled);
		if (config.kill_aura.enabled) {
			ImGui::Indent();
			ImGuiEx::SliderFloat("Range", config.kill_aura.range, 1.f, 10.f);
			ImGui::Unindent();
		}
	}

	// public void [A-Z]{11}\(boolean [A-Z]{11}\)
	//void (*VCAnimatorMove_LateTick)(Il2CppObject* _this, float tick);
	//void hVCAnimatorMove_LateTick(Il2CppObject* _this, float tick) {
	//	if (config.kill_aura.enabled) {
	//		MoleMole::BaseEntity* monster = *(MoleMole::BaseEntity**)((uintptr_t)_this + 0x28); // probably won't change
	//		if (monster->GetType() == MoleMole::EntityType::Monster) {
	//			Unity::Vector3 avatar_pos = MoleMole::EntityManager::Instance()->GetAvatar()->GetRelativePosition();
	//			Unity::Vector3 monster_pos = monster->GetRelativePosition();
	//			float distance = avatar_pos.Distance(monster_pos);
	//			if (distance <= config.kill_aura.range) {
	//				monster->SetAbsolutePosition({ monster_pos.x, 1000.f, monster_pos.z });
	//				VCAnimatorMove_AddVelocity(_this, 7, { 0.f, -1000.f, 0.f }, 0.1f, 0.1f, true, 0.1f);
	//				/**(int*)((uintptr_t)_this + Il2Cpp::Field::GetOffset(_this->klass, "_isInWater")) = 2;
	//				VCAnimatorMove_DrownWater(_this);*/
	//			}
	//		}
	//	}
	//	VCAnimatorMove_LateTick(_this, tick);
	//}

	struct ProtoVector {
		char _[0x18];
		Unity::Vector3 vector;
	};

	struct MotionInfo {
		char _[0x28];
		ProtoVector* pos1;
		ProtoVector* pos2;
		ProtoVector* pos3;
		ProtoVector* pos4;
	};

	// public FALFDBINCDK [A-Z]{11}\(uint32 [A-Z]{11}\)
	// void [A-Z]{11}\(uint32 [A-Z]{11}, [A-Z]{11} [A-Z]{11}, boolean [A-Z]{11}, uint32 [A-Z]{11}, uint32 [A-Z]{11}\)

	void (*LevelSyncCombatPlugin_RequestSceneEntityMoveReq)(Il2CppObject* _this, uint32_t entityId, MotionInfo* motionInfo, bool a1, uint32_t a2, uint32_t a3);
	void hLevelSyncCombatPlugin_RequestSceneEntityMoveReq(Il2CppObject* _this, uint32_t entityId, MotionInfo* motionInfo, bool a1, uint32_t a2, uint32_t a3) {
		if (config.kill_aura.enabled) {
			MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
			MoleMole::BaseEntity* entity = entity_manager->GetValidEntity(entityId);

			if (entity) {
				if (entity->GetType() == MoleMole::EntityType::Monster) {
					Unity::Vector3 avatar_pos = MoleMole::EntityManager::Instance()->GetAvatar()->GetRelativePosition();
					Unity::Vector3 entity_pos = entity->GetRelativePosition();
					float distance = avatar_pos.Distance(entity_pos);
					if (distance <= config.kill_aura.range) {
						Log("Entity ID: %d, Type: %d\n", entityId, (int)entity->GetType());
						entity->SetAbsolutePosition({ entity_pos.x, -1000.f, entity_pos.z });

						//Log("Entity ID: %d, Type: %d\n", entityId, (int)entity->GetType());
						/*motionInfo->pos1->vector = avatar_pos;
						motionInfo->pos2->vector = avatar_pos;
						motionInfo->pos3->vector = avatar_pos;*/
						motionInfo->pos4->vector = avatar_pos;

						MoleMole::ActorUtils::SyncEntityPos(entity, 0, 0);
					}
				}
			}
		}
		LevelSyncCombatPlugin_RequestSceneEntityMoveReq(_this, entityId, motionInfo, true, a2, a3);
	}

	void KillAura::OnInit() {
		/*MH_CreateHook(Il2Cpp::Method::GetMethodPointer(Il2Cpp::Method::Find("MoleMole", "VCAnimatorMove", "LateTick", 1)),
			(LPVOID)hVCAnimatorMove_LateTick, (LPVOID*)&VCAnimatorMove_LateTick);*/

		MH_CreateHook((void*)(g_game_base + 0xFB44AA0),
			(LPVOID)hLevelSyncCombatPlugin_RequestSceneEntityMoveReq, (LPVOID*)&LevelSyncCombatPlugin_RequestSceneEntityMoveReq);
	}
}