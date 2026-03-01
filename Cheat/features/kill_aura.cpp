#include "kill_aura.h"

namespace features
{
	void KillAura::DrawUI() {
		ImGuiEx::Checkbox("Enable kill aura", config.kill_aura.enabled);
		if (config.kill_aura.enabled) {
			ImGui::Indent();
			ImGuiEx::SliderFloat("Range", config.kill_aura.range, 1.f, 50.f);
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

	/*struct ProtoVector {
		char _[0x18];
		Unity::Vector3 vector;
	};*/

	/*struct MotionInfo {
		char _[0x18];
		ProtoVector* pos1;
		ProtoVector* pos2;
		ProtoVector* pos3;
		void* unk1;
		ProtoVector* pos4;
	};*/

	// public FALFDBINCDK [A-Z]{11}\(uint32 [A-Z]{11}\)
	// void [A-Z]{11}\(uint32 [A-Z]{11}, [A-Z]{11} [A-Z]{11}, boolean [A-Z]{11}, uint32 [A-Z]{11}, uint32 [A-Z]{11}\)

	
	void (*LevelSyncCombatPlugin_RequestSceneEntityMoveReq)(Il2CppObject* _this, uint32_t entityId, Proto::MotionInfo* motionInfo, bool a1, uint32_t a2, uint32_t a3);
	void hLevelSyncCombatPlugin_RequestSceneEntityMoveReq(Il2CppObject* _this, uint32_t entityId, Proto::MotionInfo* motionInfo, bool a1, uint32_t a2, uint32_t a3) {
		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		MoleMole::BaseEntity* ent = entity_manager->GetValidEntity(entityId);
		if (ent == entity_manager->GetAvatar()) {
			Proto::MotionState& state = motionInfo->GetMotionState();
			static bool afterDash = false;

			switch (state)
			{
			case Proto::MotionState::MotionDash:
			case Proto::MotionState::MotionClimb:
			case Proto::MotionState::MotionClimbJump:
			case Proto::MotionState::MotionStandbyToClimb:
			case Proto::MotionState::MotionSwimDash:
			case Proto::MotionState::MotionSwimIdle:
			case Proto::MotionState::MotionSwimMove:
			case Proto::MotionState::MotionSwimJump:
			case Proto::MotionState::MotionFly:
			case Proto::MotionState::MotionFight:
			case Proto::MotionState::MotionDashBeforeShake:
			case Proto::MotionState::MotionDangerDash:
				state = Proto::MotionState::MotionRun;
				break;
			case Proto::MotionState::MotionJump:
				if (afterDash)
					state = Proto::MotionState::MotionRun;
				break;
			case Proto::MotionState::MotionSkiffDash:
			case Proto::MotionState::MotionSkiffPoweredDash:
				state = Proto::MotionState::MotionSkiffNormal;
				break;
			}
			if (state != Proto::MotionState::MotionJump && state != Proto::MotionState::MotionFallOnGround)
				afterDash = state == Proto::MotionState::MotionDash;
			//Log("LevelSyncCombatPlugin_RequestSceneEntityMoveReq has been called on avatar, motionState = %d\n", state);
		}

		//if (config.kill_aura.enabled) {
		//	MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		//	MoleMole::BaseEntity* entity = entity_manager->GetValidEntity(entityId);

		//	if (entity) {
		//		if (entity->GetType() == MoleMole::EntityType::Monster) {
		//			Unity::Vector3 avatar_pos = MoleMole::EntityManager::Instance()->GetAvatar()->GetRelativePosition();
		//			Unity::Vector3 entity_pos = entity->GetRelativePosition();
		//			float distance = avatar_pos.Distance(entity_pos);
		//			if (distance <= config.kill_aura.range) {
		//				Log("Entity ID: %d, Type: %d\n", entityId, (int)entity->GetType());
		//				entity->SetAbsolutePosition({ entity_pos.x, -1000.f, entity_pos.z });

		//				/*Log("pos1 %s\n", motionInfo->pos1->vector.ToString().c_str());
		//				Log("pos2 %s\n", motionInfo->pos2->vector.ToString().c_str());
		//				Log("pos3 %s\n", motionInfo->pos3->vector.ToString().c_str());
		//				Log("pos4 %s\n", motionInfo->pos4->vector.ToString().c_str());*/

		//				motionInfo->pos2->vector = avatar_pos;

		//				MoleMole::ActorUtils::SyncEntityPos(entity, 0, 0);
		//			}
		//		}
		//	}
		//}
		LevelSyncCombatPlugin_RequestSceneEntityMoveReq(_this, entityId, motionInfo, true, a2, a3);
	}

	void (*LevelSyncCombatPlugin_TickFlushTimeAcc)(Il2CppObject* _this);
	void hLevelSyncCombatPlugin_TickFlushTimeAcc(Il2CppObject* _this) {
		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		if (!entity_manager) return;
		std::vector<MoleMole::BaseEntity*> entities = entity_manager->GetEntities();
		for (auto* entity : entities) {
			if (!entity) continue;
			if (entity->GetType() != MoleMole::EntityType::Monster) continue;

			Unity::Vector3 avatar_pos = MoleMole::EntityManager::Instance()->GetAvatar()->GetRelativePosition();
			Unity::Vector3 entity_pos = entity->GetRelativePosition();
			float distance = avatar_pos.Distance(entity_pos);
			if (distance <= config.kill_aura.range) {
				Log("[KillAura] Entity ID: %d, type: %d\n", entity->GetRuntimeID(), (int)entity->GetType(), entity->GetName()->ToCStr());
				
				entity->SetAbsolutePosition({ entity_pos.x, -1000.f, entity_pos.z });
				MoleMole::ActorUtils::SyncEntityPos(entity, 0, 0);
			}

		}
		LevelSyncCombatPlugin_TickFlushTimeAcc(_this);
	}

	void KillAura::OnInit() {
		MH_CreateHook((LPVOID)(Mem::Signature(
			"56 48 83 EC ? 0F 29 74 24 ? 48 89 CE 80 3D ? ? ? ? 00 75 ? F3 0F 10 76 ? E8 ? ? ? ? F3 0F 58 C6 F3 0F 11 46 ? E8 ? ? ? ? 0F 57 C0").Scan()),
			(LPVOID)hLevelSyncCombatPlugin_TickFlushTimeAcc, (LPVOID*)&LevelSyncCombatPlugin_TickFlushTimeAcc);

		MH_CreateHook((LPVOID)(Mem::Signature("41 57 41 56 41 54 56 57 55 53 48 83 EC ? 45 89 CF 4D 89 C6 89 D5 48 89 CE").Scan()),
			(LPVOID)hLevelSyncCombatPlugin_RequestSceneEntityMoveReq, (LPVOID*)&LevelSyncCombatPlugin_RequestSceneEntityMoveReq);
	}
}