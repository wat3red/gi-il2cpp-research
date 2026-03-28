#include "kill_aura.h"

namespace features
{
	void KillAura::DrawUI() {
		ImGuiEx::Checkbox("Enable kill aura", config.kill_aura.enabled);
		if (config.kill_aura.enabled) {
			ImGui::Indent();
			ImGuiEx::SliderFloat("Range", config.kill_aura.range, 1.f, 50.f);
			ImGuiEx::SliderFloat("Delay", config.kill_aura.delay, 1.f, 100.f);
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
	// 
	// public [A-Z]{11} [A-Z]{11}\(string [A-Z]{11}\)

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



	std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> entity_spawn_time;

	void (*LevelSyncCombatPlugin_TickFlushTimeAcc)(Il2CppObject* _this);
	void hLevelSyncCombatPlugin_TickFlushTimeAcc(Il2CppObject* _this) {
		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		if (!entity_manager) return;

		if (!config.kill_aura.enabled) return;

		auto now = std::chrono::steady_clock::now();

		std::vector<MoleMole::BaseEntity*> entities = entity_manager->GetEntities();
		for (auto* entity : entities) {
			if (!entity) continue;
			if (entity->GetType() != MoleMole::EntityType::Monster) continue;

			uint32_t id = entity->GetRuntimeID();

			// writing down the spawn time of the entity
			if (entity_spawn_time.find(id) == entity_spawn_time.end()) {
				entity_spawn_time[id] = now;
				continue;
			}

			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - entity_spawn_time[id]).count();

			if (elapsed < config.kill_aura.delay)
				continue;

			Unity::Vector3 avatar_pos = entity_manager->GetAvatar()->GetRelativePosition();
			Unity::Vector3 entity_pos = entity->GetRelativePosition();

			float distance = avatar_pos.Distance(entity_pos);
			if (distance <= config.kill_aura.range) {

				Log("[KillAura Delay] ID: %d | time: %.2f\n", id, elapsed);

				entity->SetAbsolutePosition({ entity_pos.x, -1000.f, entity_pos.z });
				MoleMole::ActorUtils::SyncEntityPos(entity, 0, 0);

				// Удаляем из карты, чтобы не копился мусор
				entity_spawn_time.erase(id);
			}
		}

		LevelSyncCombatPlugin_TickFlushTimeAcc(_this);
	}



	//public void [a-z]{11}\(uint32 [a-z]{11}, DCDKGOKNGIK [a-z]{11}\)


	// 0xe0948c0
	//  BCAPHHFEMJK is LCBaseCombat 
	// MoleMole.AttackResult: DCDKGOKNGIK

	struct SafeFloat {
		int64_t a;
		int64_t b;

		float GetValue() {
			return ((float(*)(SafeFloat*))(g_game_base + 0x82B70))(this);
		}

		void SetValue(float v) {
			return ((void(*)(SafeFloat*, float))(g_game_base + 0x82C60))(this, v);
		}
	};

	struct unknown {
		int64_t KHDLLCJMMON; // 0x0
		int64_t JBHCMGLEBBB; // 0x8
	};

	struct CombatProperty {
		void* ptr_1;
		void* ptr_2;
		void* ptr_3;
		void* ptr_4;
		void* ptr_5;
		void* ptr_6;
		void* ptr_7;
		void* ptr_8;
		void* ptr_9;
		void* ptr_10;
		void* ptr_11;
		SafeFloat safe_float_1;
		SafeFloat safe_float_2;
		SafeFloat safe_float_3;
		SafeFloat safe_float_4;
		SafeFloat safe_float_5;
		SafeFloat safe_float_6;
		SafeFloat safe_float_7;
		SafeFloat safe_float_8;
		SafeFloat safe_float_9;
		SafeFloat safe_float_10;
		SafeFloat safe_float_11;
		SafeFloat safe_float_12;
		SafeFloat safe_float_13;
		SafeFloat safe_float_14;
		SafeFloat safe_float_15;
		SafeFloat safe_float_16;
		SafeFloat safe_float_17;
		SafeFloat safe_float_18;
		SafeFloat safe_float_19;
		SafeFloat safe_float_20;
		SafeFloat safe_float_21;
		SafeFloat safe_float_22;
		SafeFloat safe_float_23;
		SafeFloat safe_float_24;
		SafeFloat safe_float_25;
		SafeFloat safe_float_26;
		SafeFloat safe_float_27;
		SafeFloat safe_float_28;
		SafeFloat safe_float_29;
		SafeFloat safe_float_30;
		SafeFloat safe_float_31;
		SafeFloat safe_float_32;
		SafeFloat safe_float_33;
		unknown unknown_1;
		SafeFloat safe_float_34; // crit damage
		SafeFloat safe_float_35;
		SafeFloat safe_float_36;
		SafeFloat safe_float_37;
		SafeFloat safe_float_38;
		SafeFloat safe_float_39;
		SafeFloat safe_float_40;
		SafeFloat safe_float_41;
		SafeFloat safe_float_42;
		SafeFloat safe_float_43;
		SafeFloat safe_float_44;
		SafeFloat safe_float_45; // attackBase
		SafeFloat safe_float_46;
		SafeFloat safe_float_47;
		SafeFloat safe_float_48;
		SafeFloat safe_float_49;
		SafeFloat safe_float_50;
		SafeFloat safe_float_51;
		SafeFloat safe_float_52;
		SafeFloat safe_float_53;
		SafeFloat safe_float_54;
		SafeFloat safe_float_55;
		SafeFloat safe_float_56;
		SafeFloat safe_float_57;
		SafeFloat safe_float_58;
		SafeFloat safe_float_59;
		SafeFloat safe_float_60;
		SafeFloat safe_float_61;
		SafeFloat safe_float_62;
		SafeFloat safe_float_63;
		SafeFloat safe_float_64;
		SafeFloat safe_float_65;
		SafeFloat safe_float_66;
		SafeFloat safe_float_67;
		SafeFloat safe_float_68;
		SafeFloat safe_float_69;
		SafeFloat safe_float_70;
		SafeFloat safe_float_71;
		SafeFloat safe_float_72;
		SafeFloat safe_float_73;
		SafeFloat safe_float_74;
		SafeFloat safe_float_75;
		SafeFloat safe_float_76;
		SafeFloat safe_float_77;
		SafeFloat safe_float_78;
		SafeFloat safe_float_79;
		SafeFloat safe_float_80;
		SafeFloat safe_float_81;
		SafeFloat safe_float_82;
		SafeFloat safe_float_83;
		SafeFloat safe_float_84;
		SafeFloat safe_float_85;
		SafeFloat safe_float_86;
		SafeFloat safe_float_87;
		SafeFloat safe_float_88;
		SafeFloat safe_float_89;
		bool useAbilityProperty;
		int32_t elemType;
		SafeFloat safe_float_90;
		SafeFloat safe_float_91;
		SafeFloat safe_float_92; // crit rate
		SafeFloat safe_float_93;
		SafeFloat safe_float_94;
		SafeFloat safe_float_95;
		SafeFloat safe_float_96;
		SafeFloat safe_float_97;
		SafeFloat safe_float_98;
		SafeFloat safe_float_99;
		SafeFloat safe_float_100;
		SafeFloat safe_float_101;
		SafeFloat safe_float_102;
		SafeFloat safe_float_103;
		SafeFloat safe_float_104;
		SafeFloat safe_float_105;
		SafeFloat safe_float_106;
		SafeFloat safe_float_107;
		SafeFloat safe_float_108;
		SafeFloat safe_float_109;
		SafeFloat safe_float_110;
		SafeFloat safe_float_111;
		SafeFloat safe_float_112;
		SafeFloat safe_float_113;
	};

	void* (*AttackResult_CreateAttackResult)(MoleMole::BaseEntity*, void*, void*, Il2CppString*, void*, void*,
		Unity::Vector3, Unity::Vector3, uint32_t, void*, void*, uint32_t, void*);

	void* hAttackResult_CreateAttackResult(MoleMole::BaseEntity* attacker, void* attackee, void* attackInfo,
		Il2CppString* animEventId, void* abilityId, void* damageIdx,
		Unity::Vector3 hitPoint, Unity::Vector3 hitForward, uint32_t hitSceneId,
		void* collider, void* targetType, uint32_t someId, void* extra) {

		void* result = AttackResult_CreateAttackResult(
			attacker,
			attackee,
			attackInfo,
			animEventId,
			abilityId,
			damageIdx,
			hitPoint,
			hitForward,
			hitSceneId,
			collider,
			targetType,
			someId,
			extra
		);

		// may be public PAGADEGKDNA

		if (!result) return result;

		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		if (!entity_manager) return result;

		Log("attacker: %p\n", attacker);
		Log("attacker name: %s\n", attacker->GetName()->ToCStr());
		Log("entity_manager->GetAvatar(): %p\n", entity_manager->GetAvatar());

		if (attacker != entity_manager->GetAvatar()) return result;

		CombatProperty* combatProp1 = *(CombatProperty**)((uintptr_t)result + 0x40);


		//CombatProperty* combatProp2 = *(CombatProperty**)((uintptr_t)result + 0xf8);

		// Log all SafeFloat values from combatProp1
		if (combatProp1) {

			//combatProp1->safe_float_45.SetValue(3000.f);
			combatProp1->safe_float_34.SetValue(5.f);
			combatProp1->safe_float_92.SetValue(1.f);

			Log("=== combatProp1 SafeFloat values ===\n");
			Log("safe_float_1: %f\n", combatProp1->safe_float_1.GetValue());
			Log("safe_float_2: %f\n", combatProp1->safe_float_2.GetValue());
			Log("safe_float_3: %f\n", combatProp1->safe_float_3.GetValue());
			Log("safe_float_4: %f\n", combatProp1->safe_float_4.GetValue());
			Log("safe_float_5: %f\n", combatProp1->safe_float_5.GetValue());
			Log("safe_float_6: %f\n", combatProp1->safe_float_6.GetValue());
			Log("safe_float_7: %f\n", combatProp1->safe_float_7.GetValue());
			Log("safe_float_8: %f\n", combatProp1->safe_float_8.GetValue());
			Log("safe_float_9: %f\n", combatProp1->safe_float_9.GetValue());
			Log("safe_float_10: %f\n", combatProp1->safe_float_10.GetValue());
			Log("safe_float_11: %f\n", combatProp1->safe_float_11.GetValue());
			Log("safe_float_12: %f\n", combatProp1->safe_float_12.GetValue());
			Log("safe_float_13: %f\n", combatProp1->safe_float_13.GetValue());
			Log("safe_float_14: %f\n", combatProp1->safe_float_14.GetValue());
			Log("safe_float_15: %f\n", combatProp1->safe_float_15.GetValue());
			Log("safe_float_16: %f\n", combatProp1->safe_float_16.GetValue());
			Log("safe_float_17: %f\n", combatProp1->safe_float_17.GetValue());
			Log("safe_float_18: %f\n", combatProp1->safe_float_18.GetValue());
			Log("safe_float_19: %f\n", combatProp1->safe_float_19.GetValue());
			Log("safe_float_20: %f\n", combatProp1->safe_float_20.GetValue());
			Log("safe_float_21: %f\n", combatProp1->safe_float_21.GetValue());
			Log("safe_float_22: %f\n", combatProp1->safe_float_22.GetValue());
			Log("safe_float_23: %f\n", combatProp1->safe_float_23.GetValue());
			Log("safe_float_24: %f\n", combatProp1->safe_float_24.GetValue());
			Log("safe_float_25: %f\n", combatProp1->safe_float_25.GetValue());
			Log("safe_float_26: %f\n", combatProp1->safe_float_26.GetValue());
			Log("safe_float_27: %f\n", combatProp1->safe_float_27.GetValue());
			Log("safe_float_28: %f\n", combatProp1->safe_float_28.GetValue());
			Log("safe_float_29: %f\n", combatProp1->safe_float_29.GetValue());
			Log("safe_float_30: %f\n", combatProp1->safe_float_30.GetValue());
			Log("safe_float_31: %f\n", combatProp1->safe_float_31.GetValue());
			Log("safe_float_32: %f\n", combatProp1->safe_float_32.GetValue());
			Log("safe_float_33: %f\n", combatProp1->safe_float_33.GetValue());
			Log("safe_float_34: %f\n", combatProp1->safe_float_34.GetValue());
			Log("safe_float_35: %f\n", combatProp1->safe_float_35.GetValue());
			Log("safe_float_36: %f\n", combatProp1->safe_float_36.GetValue());
			Log("safe_float_37: %f\n", combatProp1->safe_float_37.GetValue());
			Log("safe_float_38: %f\n", combatProp1->safe_float_38.GetValue());
			Log("safe_float_39: %f\n", combatProp1->safe_float_39.GetValue());
			Log("safe_float_40: %f\n", combatProp1->safe_float_40.GetValue());
			Log("safe_float_41: %f\n", combatProp1->safe_float_41.GetValue());
			Log("safe_float_42: %f\n", combatProp1->safe_float_42.GetValue());
			Log("safe_float_43: %f\n", combatProp1->safe_float_43.GetValue());
			Log("safe_float_44: %f\n", combatProp1->safe_float_44.GetValue());
			Log("safe_float_45: %f\n", combatProp1->safe_float_45.GetValue());
			Log("safe_float_46: %f\n", combatProp1->safe_float_46.GetValue());
			Log("safe_float_47: %f\n", combatProp1->safe_float_47.GetValue());
			Log("safe_float_48: %f\n", combatProp1->safe_float_48.GetValue());
			Log("safe_float_49: %f\n", combatProp1->safe_float_49.GetValue());
			Log("safe_float_50: %f\n", combatProp1->safe_float_50.GetValue());
			Log("safe_float_51: %f\n", combatProp1->safe_float_51.GetValue());
			Log("safe_float_52: %f\n", combatProp1->safe_float_52.GetValue());
			Log("safe_float_53: %f\n", combatProp1->safe_float_53.GetValue());
			Log("safe_float_54: %f\n", combatProp1->safe_float_54.GetValue());
			Log("safe_float_55: %f\n", combatProp1->safe_float_55.GetValue());
			Log("safe_float_56: %f\n", combatProp1->safe_float_56.GetValue());
			Log("safe_float_57: %f\n", combatProp1->safe_float_57.GetValue());
			Log("safe_float_58: %f\n", combatProp1->safe_float_58.GetValue());
			Log("safe_float_59: %f\n", combatProp1->safe_float_59.GetValue());
			Log("safe_float_60: %f\n", combatProp1->safe_float_60.GetValue());
			Log("safe_float_61: %f\n", combatProp1->safe_float_61.GetValue());
			Log("safe_float_62: %f\n", combatProp1->safe_float_62.GetValue());
			Log("safe_float_63: %f\n", combatProp1->safe_float_63.GetValue());
			Log("safe_float_64: %f\n", combatProp1->safe_float_64.GetValue());
			Log("safe_float_65: %f\n", combatProp1->safe_float_65.GetValue());
			Log("safe_float_66: %f\n", combatProp1->safe_float_66.GetValue());
			Log("safe_float_67: %f\n", combatProp1->safe_float_67.GetValue());
			Log("safe_float_68: %f\n", combatProp1->safe_float_68.GetValue());
			Log("safe_float_69: %f\n", combatProp1->safe_float_69.GetValue());
			Log("safe_float_70: %f\n", combatProp1->safe_float_70.GetValue());
			Log("safe_float_71: %f\n", combatProp1->safe_float_71.GetValue());
			Log("safe_float_72: %f\n", combatProp1->safe_float_72.GetValue());
			Log("safe_float_73: %f\n", combatProp1->safe_float_73.GetValue());
			Log("safe_float_74: %f\n", combatProp1->safe_float_74.GetValue());
			Log("safe_float_75: %f\n", combatProp1->safe_float_75.GetValue());
			Log("safe_float_76: %f\n", combatProp1->safe_float_76.GetValue());
			Log("safe_float_77: %f\n", combatProp1->safe_float_77.GetValue());
			Log("safe_float_78: %f\n", combatProp1->safe_float_78.GetValue());
			Log("safe_float_79: %f\n", combatProp1->safe_float_79.GetValue());
			Log("safe_float_80: %f\n", combatProp1->safe_float_80.GetValue());
			Log("safe_float_81: %f\n", combatProp1->safe_float_81.GetValue());
			Log("safe_float_82: %f\n", combatProp1->safe_float_82.GetValue());
			Log("safe_float_83: %f\n", combatProp1->safe_float_83.GetValue());
			Log("safe_float_84: %f\n", combatProp1->safe_float_84.GetValue());
			Log("safe_float_85: %f\n", combatProp1->safe_float_85.GetValue());
			Log("safe_float_86: %f\n", combatProp1->safe_float_86.GetValue());
			Log("safe_float_87: %f\n", combatProp1->safe_float_87.GetValue());
			Log("safe_float_88: %f\n", combatProp1->safe_float_88.GetValue());
			Log("safe_float_89: %f\n", combatProp1->safe_float_89.GetValue());
			Log("safe_float_90: %f\n", combatProp1->safe_float_90.GetValue());
			Log("safe_float_91: %f\n", combatProp1->safe_float_91.GetValue());
			Log("safe_float_92: %f\n", combatProp1->safe_float_92.GetValue());
			Log("safe_float_93: %f\n", combatProp1->safe_float_93.GetValue());
			Log("safe_float_94: %f\n", combatProp1->safe_float_94.GetValue());
			Log("safe_float_95: %f\n", combatProp1->safe_float_95.GetValue());
			Log("safe_float_96: %f\n", combatProp1->safe_float_96.GetValue());
			Log("safe_float_97: %f\n", combatProp1->safe_float_97.GetValue());
			Log("safe_float_98: %f\n", combatProp1->safe_float_98.GetValue());
			Log("safe_float_99: %f\n", combatProp1->safe_float_99.GetValue());
			Log("safe_float_100: %f\n", combatProp1->safe_float_100.GetValue());
			Log("safe_float_101: %f\n", combatProp1->safe_float_101.GetValue());
		}

		// Log all SafeFloat values from combatProp2
		/*if (combatProp2) {
			Log("=== combatProp2 SafeFloat values ===\n");
			Log("safe_float_1: %f\n", combatProp2->safe_float_1.GetValue());
			Log("safe_float_2: %f\n", combatProp2->safe_float_2.GetValue());
			Log("safe_float_3: %f\n", combatProp2->safe_float_3.GetValue());
			Log("safe_float_4: %f\n", combatProp2->safe_float_4.GetValue());
			Log("safe_float_5: %f\n", combatProp2->safe_float_5.GetValue());
			Log("safe_float_6: %f\n", combatProp2->safe_float_6.GetValue());
			Log("safe_float_7: %f\n", combatProp2->safe_float_7.GetValue());
			Log("safe_float_8: %f\n", combatProp2->safe_float_8.GetValue());
			Log("safe_float_9: %f\n", combatProp2->safe_float_9.GetValue());
			Log("safe_float_10: %f\n", combatProp2->safe_float_10.GetValue());
			Log("safe_float_11: %f\n", combatProp2->safe_float_11.GetValue());
			Log("safe_float_12: %f\n", combatProp2->safe_float_12.GetValue());
			Log("safe_float_13: %f\n", combatProp2->safe_float_13.GetValue());
			Log("safe_float_14: %f\n", combatProp2->safe_float_14.GetValue());
			Log("safe_float_15: %f\n", combatProp2->safe_float_15.GetValue());
			Log("safe_float_16: %f\n", combatProp2->safe_float_16.GetValue());
			Log("safe_float_17: %f\n", combatProp2->safe_float_17.GetValue());
			Log("safe_float_18: %f\n", combatProp2->safe_float_18.GetValue());
			Log("safe_float_19: %f\n", combatProp2->safe_float_19.GetValue());
			Log("safe_float_20: %f\n", combatProp2->safe_float_20.GetValue());
			Log("safe_float_21: %f\n", combatProp2->safe_float_21.GetValue());
			Log("safe_float_22: %f\n", combatProp2->safe_float_22.GetValue());
			Log("safe_float_23: %f\n", combatProp2->safe_float_23.GetValue());
			Log("safe_float_24: %f\n", combatProp2->safe_float_24.GetValue());
			Log("safe_float_25: %f\n", combatProp2->safe_float_25.GetValue());
			Log("safe_float_26: %f\n", combatProp2->safe_float_26.GetValue());
			Log("safe_float_27: %f\n", combatProp2->safe_float_27.GetValue());
			Log("safe_float_28: %f\n", combatProp2->safe_float_28.GetValue());
			Log("safe_float_29: %f\n", combatProp2->safe_float_29.GetValue());
			Log("safe_float_30: %f\n", combatProp2->safe_float_30.GetValue());
			Log("safe_float_31: %f\n", combatProp2->safe_float_31.GetValue());
			Log("safe_float_32: %f\n", combatProp2->safe_float_32.GetValue());
			Log("safe_float_33: %f\n", combatProp2->safe_float_33.GetValue());
			Log("safe_float_34: %f\n", combatProp2->safe_float_34.GetValue());
			Log("safe_float_35: %f\n", combatProp2->safe_float_35.GetValue());
			Log("safe_float_36: %f\n", combatProp2->safe_float_36.GetValue());
			Log("safe_float_37: %f\n", combatProp2->safe_float_37.GetValue());
			Log("safe_float_38: %f\n", combatProp2->safe_float_38.GetValue());
			Log("safe_float_39: %f\n", combatProp2->safe_float_39.GetValue());
			Log("safe_float_40: %f\n", combatProp2->safe_float_40.GetValue());
			Log("safe_float_41: %f\n", combatProp2->safe_float_41.GetValue());
			Log("safe_float_42: %f\n", combatProp2->safe_float_42.GetValue());
			Log("safe_float_43: %f\n", combatProp2->safe_float_43.GetValue());
			Log("safe_float_44: %f\n", combatProp2->safe_float_44.GetValue());
			Log("safe_float_45: %f\n", combatProp2->safe_float_45.GetValue());
			Log("safe_float_46: %f\n", combatProp2->safe_float_46.GetValue());
			Log("safe_float_47: %f\n", combatProp2->safe_float_47.GetValue());
			Log("safe_float_48: %f\n", combatProp2->safe_float_48.GetValue());
			Log("safe_float_49: %f\n", combatProp2->safe_float_49.GetValue());
			Log("safe_float_50: %f\n", combatProp2->safe_float_50.GetValue());
			Log("safe_float_51: %f\n", combatProp2->safe_float_51.GetValue());
			Log("safe_float_52: %f\n", combatProp2->safe_float_52.GetValue());
			Log("safe_float_53: %f\n", combatProp2->safe_float_53.GetValue());
			Log("safe_float_54: %f\n", combatProp2->safe_float_54.GetValue());
			Log("safe_float_55: %f\n", combatProp2->safe_float_55.GetValue());
			Log("safe_float_56: %f\n", combatProp2->safe_float_56.GetValue());
			Log("safe_float_57: %f\n", combatProp2->safe_float_57.GetValue());
			Log("safe_float_58: %f\n", combatProp2->safe_float_58.GetValue());
			Log("safe_float_59: %f\n", combatProp2->safe_float_59.GetValue());
			Log("safe_float_60: %f\n", combatProp2->safe_float_60.GetValue());
			Log("safe_float_61: %f\n", combatProp2->safe_float_61.GetValue());
			Log("safe_float_62: %f\n", combatProp2->safe_float_62.GetValue());
			Log("safe_float_63: %f\n", combatProp2->safe_float_63.GetValue());
			Log("safe_float_64: %f\n", combatProp2->safe_float_64.GetValue());
			Log("safe_float_65: %f\n", combatProp2->safe_float_65.GetValue());
			Log("safe_float_66: %f\n", combatProp2->safe_float_66.GetValue());
			Log("safe_float_67: %f\n", combatProp2->safe_float_67.GetValue());
			Log("safe_float_68: %f\n", combatProp2->safe_float_68.GetValue());
			Log("safe_float_69: %f\n", combatProp2->safe_float_69.GetValue());
			Log("safe_float_70: %f\n", combatProp2->safe_float_70.GetValue());
			Log("safe_float_71: %f\n", combatProp2->safe_float_71.GetValue());
			Log("safe_float_72: %f\n", combatProp2->safe_float_72.GetValue());
			Log("safe_float_73: %f\n", combatProp2->safe_float_73.GetValue());
			Log("safe_float_74: %f\n", combatProp2->safe_float_74.GetValue());
			Log("safe_float_75: %f\n", combatProp2->safe_float_75.GetValue());
			Log("safe_float_76: %f\n", combatProp2->safe_float_76.GetValue());
			Log("safe_float_77: %f\n", combatProp2->safe_float_77.GetValue());
			Log("safe_float_78: %f\n", combatProp2->safe_float_78.GetValue());
			Log("safe_float_79: %f\n", combatProp2->safe_float_79.GetValue());
			Log("safe_float_80: %f\n", combatProp2->safe_float_80.GetValue());
			Log("safe_float_81: %f\n", combatProp2->safe_float_81.GetValue());
			Log("safe_float_82: %f\n", combatProp2->safe_float_82.GetValue());
			Log("safe_float_83: %f\n", combatProp2->safe_float_83.GetValue());
			Log("safe_float_84: %f\n", combatProp2->safe_float_84.GetValue());
			Log("safe_float_85: %f\n", combatProp2->safe_float_85.GetValue());
			Log("safe_float_86: %f\n", combatProp2->safe_float_86.GetValue());
			Log("safe_float_87: %f\n", combatProp2->safe_float_87.GetValue());
			Log("safe_float_88: %f\n", combatProp2->safe_float_88.GetValue());
			Log("safe_float_89: %f\n", combatProp2->safe_float_89.GetValue());
			Log("safe_float_90: %f\n", combatProp2->safe_float_90.GetValue());
			Log("safe_float_91: %f\n", combatProp2->safe_float_91.GetValue());
			Log("safe_float_92: %f\n", combatProp2->safe_float_92.GetValue());
			Log("safe_float_93: %f\n", combatProp2->safe_float_93.GetValue());
			Log("safe_float_94: %f\n", combatProp2->safe_float_94.GetValue());
			Log("safe_float_95: %f\n", combatProp2->safe_float_95.GetValue());
			Log("safe_float_96: %f\n", combatProp2->safe_float_96.GetValue());
			Log("safe_float_97: %f\n", combatProp2->safe_float_97.GetValue());
			Log("safe_float_98: %f\n", combatProp2->safe_float_98.GetValue());
			Log("safe_float_99: %f\n", combatProp2->safe_float_99.GetValue());
			Log("safe_float_100: %f\n", combatProp2->safe_float_100.GetValue());
			Log("safe_float_101: %f\n", combatProp2->safe_float_101.GetValue());
		}*/


		//Log("0x10:%s\n", (*(Il2CppString**)((uintptr_t)result + 0x10))->ToCStr());
		//Log("0x18:%s\n", (*(Il2CppString**)((uintptr_t)result + 0x18))->ToCStr());
		//Log("0x20:%s\n", (*(Il2CppString**)((uintptr_t)result + 0x20))->ToCStr());
		//Log("0x48:%s\n", (*(Il2CppString**)((uintptr_t)result + 0x48))->ToCStr());
		//Log("0x60:%s\n", (*(Il2CppString**)((uintptr_t)result + 0x60))->ToCStr());


		//Log("0x120:%f\n", *(float*)((uintptr_t)result + 0x120));
		//Log("0x13C:%f\n", *(float*)((uintptr_t)result + 0x13C));
		//Log("0x140:%f\n", *(float*)((uintptr_t)result + 0x140));
		//Log("0x158:%f\n", *(float*)((uintptr_t)result + 0x158));
		//Log("0x1A8:%f\n", *(float*)((uintptr_t)result + 0x1A8)); // bulletFlyTime

		//Log("0x244:%f\n", *(float*)((uintptr_t)result + 0x244));
		//Log("0x248:%f\n", *(float*)((uintptr_t)result + 0x248));
		//Log("0x250:%f\n", *(float*)((uintptr_t)result + 0x250));
		//Log("0x258:%f\n", *(float*)((uintptr_t)result + 0x258));
		//Log("0x280:%f\n", *(float*)((uintptr_t)result + 0x280));

		//float* damage = (float*)((uintptr_t)result + 0x244);
		//*damage *= 100.0f; // x100 урон

		return result;
	}

	//public static DCDKGOKNGIK GHMBIPNAOFB(HCJGEEJOFPB LAEJMBLFEGE, PEKADNDOOPD JICLEPBKFHN, CCAJCBEDJGG NDHBNLEJEHD, String BPCAIHHNKFI, Nullable<LDOIOJNGDNL> DHDLIPNCMGI, Nullable<UInt32> LDPACEIIEBO, Vector3 OJJFGNBJPON, Vector3 IMNEIEIKCKH, UInt32 GFMAOODBNHL, Collider KBBDOLJEIDK, Nullable<TargetType> JLOMDMLCFIP, UInt32 AKCEEJNDNDL, Nullable<EPPDKKLLHPI> MOMDEDDLIEM); // RVA: 0x80E9AD0

	//enum class FightPropType : int32_t {
	//	FIGHT_PROP_NONE,
	//	FIGHT_PROP_BASE_HP,
	//	FIGHT_PROP_HP,
	//	FIGHT_PROP_HP_PERCENT,
	//	FIGHT_PROP_BASE_ATTACK,
	//	FIGHT_PROP_ATTACK,
	//	FIGHT_PROP_ATTACK_PERCENT,
	//	FIGHT_PROP_BASE_DEFENSE,
	//	FIGHT_PROP_DEFENSE,
	//	FIGHT_PROP_DEFENSE_PERCENT,
	//	FIGHT_PROP_BASE_SPEED,
	//	FIGHT_PROP_SPEED_PERCENT,
	//	FIGHT_PROP_HP_MP_PERCENT,
	//	FIGHT_PROP_ATTACK_MP_PERCENT,
	//	FIGHT_PROP_CRITICAL,
	//	FIGHT_PROP_ANTI_CRITICAL,
	//	FIGHT_PROP_CRITICAL_HURT,
	//	FIGHT_PROP_CHARGE_EFFICIENCY,
	//	FIGHT_PROP_ADD_HURT,
	//	FIGHT_PROP_SUB_HURT,
	//	FIGHT_PROP_HEAL_ADD,
	//	FIGHT_PROP_HEALED_ADD,
	//	FIGHT_PROP_ELEMENT_MASTERY,
	//	FIGHT_PROP_PHYSICAL_SUB_HURT,
	//	FIGHT_PROP_PHYSICAL_ADD_HURT,
	//	FIGHT_PROP_DEFENCE_IGNORE_RATIO,
	//	FIGHT_PROP_DEFENCE_IGNORE_DELTA,
	//	FIGHT_PROP_FIRE_ADD_HURT,
	//	FIGHT_PROP_ELEC_ADD_HURT,
	//	FIGHT_PROP_WATER_ADD_HURT,
	//	FIGHT_PROP_GRASS_ADD_HURT,
	//	FIGHT_PROP_WIND_ADD_HURT,
	//	FIGHT_PROP_ROCK_ADD_HURT,
	//	FIGHT_PROP_ICE_ADD_HURT,
	//	FIGHT_PROP_HIT_HEAD_ADD_HURT,
	//	FIGHT_PROP_FIRE_SUB_HURT,
	//	FIGHT_PROP_ELEC_SUB_HURT,
	//	FIGHT_PROP_WATER_SUB_HURT,
	//	FIGHT_PROP_GRASS_SUB_HURT,
	//	FIGHT_PROP_WIND_SUB_HURT,
	//	FIGHT_PROP_ROCK_SUB_HURT,
	//	FIGHT_PROP_ICE_SUB_HURT,
	//	FIGHT_PROP_EFFECT_HIT,
	//	FIGHT_PROP_EFFECT_RESIST,
	//	FIGHT_PROP_FREEZE_RESIST,
	//	FIGHT_PROP_DIZZY_RESIST,
	//	FIGHT_PROP_FREEZE_SHORTEN,
	//	FIGHT_PROP_DIZZY_SHORTEN,
	//	FIGHT_PROP_MAX_FIRE_ENERGY,
	//	FIGHT_PROP_MAX_ELEC_ENERGY,
	//	FIGHT_PROP_MAX_WATER_ENERGY,
	//	FIGHT_PROP_MAX_GRASS_ENERGY,
	//	FIGHT_PROP_MAX_WIND_ENERGY,
	//	FIGHT_PROP_MAX_ICE_ENERGY,
	//	FIGHT_PROP_MAX_ROCK_ENERGY,
	//	FIGHT_PROP_MAX_SPECIAL_ENERGY,
	//	FIGHT_PROP_START_SPECIAL_ENERGY,
	//	FIGHT_PROP_SKILL_CD_MINUS_RATIO,
	//	FIGHT_PROP_SHIELD_COST_MINUS_RATIO,
	//	FIGHT_PROP_BASE_ENMITY_MULTIPLIER,
	//	FIGHT_PROP_ENMITY_MULTIPLIER_PERCENT,
	//	FIGHT_PROP_ENMITY_MULTIPLIER,
	//	FIGHT_PROP_CUR_FIRE_ENERGY,
	//	FIGHT_PROP_CUR_ELEC_ENERGY,
	//	FIGHT_PROP_CUR_WATER_ENERGY,
	//	FIGHT_PROP_CUR_GRASS_ENERGY,
	//	FIGHT_PROP_CUR_WIND_ENERGY,
	//	FIGHT_PROP_CUR_ICE_ENERGY,
	//	FIGHT_PROP_CUR_ROCK_ENERGY,
	//	FIGHT_PROP_CUR_SPECIAL_ENERGY,
	//	FIGHT_PROP_CUR_HP,
	//	FIGHT_PROP_MAX_HP,
	//	FIGHT_PROP_CUR_ATTACK,
	//	FIGHT_PROP_CUR_DEFENSE,
	//	FIGHT_PROP_CUR_SPEED,
	//	FIGHT_PROP_CUR_HP_DEBTS,
	//	FIGHT_PROP_CUR_HP_PAID_DEBTS,
	//	FIGHT_PROP_CUR_NATLAN_HP,
	//	FIGHT_PROP_CUR_ENMITY_MULTIPLIER,
	//	FIGHT_PROP_NONEXTRA_ATTACK,
	//	FIGHT_PROP_NONEXTRA_DEFENSE,
	//	FIGHT_PROP_NONEXTRA_CRITICAL,
	//	FIGHT_PROP_NONEXTRA_ANTI_CRITICAL,
	//	FIGHT_PROP_NONEXTRA_CRITICAL_HURT,
	//	FIGHT_PROP_NONEXTRA_CHARGE_EFFICIENCY,
	//	FIGHT_PROP_NONEXTRA_ELEMENT_MASTERY,
	//	FIGHT_PROP_NONEXTRA_PHYSICAL_SUB_HURT,
	//	FIGHT_PROP_NONEXTRA_FIRE_ADD_HURT,
	//	FIGHT_PROP_NONEXTRA_ELEC_ADD_HURT,
	//	FIGHT_PROP_NONEXTRA_WATER_ADD_HURT,
	//	FIGHT_PROP_NONEXTRA_GRASS_ADD_HURT,
	//	FIGHT_PROP_NONEXTRA_WIND_ADD_HURT,
	//	FIGHT_PROP_NONEXTRA_ROCK_ADD_HURT,
	//	FIGHT_PROP_NONEXTRA_ICE_ADD_HURT,
	//	FIGHT_PROP_NONEXTRA_FIRE_SUB_HURT,
	//	FIGHT_PROP_NONEXTRA_ELEC_SUB_HURT,
	//	FIGHT_PROP_NONEXTRA_WATER_SUB_HURT,
	//	FIGHT_PROP_NONEXTRA_GRASS_SUB_HURT,
	//	FIGHT_PROP_NONEXTRA_WIND_SUB_HURT,
	//	FIGHT_PROP_NONEXTRA_ROCK_SUB_HURT,
	//	FIGHT_PROP_NONEXTRA_ICE_SUB_HURT,
	//	FIGHT_PROP_NONEXTRA_SKILL_CD_MINUS_RATIO,
	//	FIGHT_PROP_NONEXTRA_SHIELD_COST_MINUS_RATIO,
	//	FIGHT_PROP_NONEXTRA_PHYSICAL_ADD_HURT,
	//	FIGHT_PROP_BASE_ELEM_REACT_CRITICAL,
	//	FIGHT_PROP_BASE_ELEM_REACT_CRITICAL_HURT,
	//	FIGHT_PROP_ELEM_REACT_CRITICAL,
	//	FIGHT_PROP_ELEM_REACT_CRITICAL_HURT,
	//	FIGHT_PROP_ELEM_REACT_EXPLODE_CRITICAL,
	//	FIGHT_PROP_ELEM_REACT_EXPLODE_CRITICAL_HURT,
	//	FIGHT_PROP_ELEM_REACT_SWIRL_CRITICAL,
	//	FIGHT_PROP_ELEM_REACT_SWIRL_CRITICAL_HURT,
	//	FIGHT_PROP_ELEM_REACT_ELECTRIC_CRITICAL,
	//	FIGHT_PROP_ELEM_REACT_ELECTRIC_CRITICAL_HURT,
	//	FIGHT_PROP_ELEM_REACT_SCONDUCT_CRITICAL,
	//	FIGHT_PROP_ELEM_REACT_SCONDUCT_CRITICAL_HURT,
	//	FIGHT_PROP_ELEM_REACT_BURN_CRITICAL,
	//	FIGHT_PROP_ELEM_REACT_BURN_CRITICAL_HURT,
	//	FIGHT_PROP_ELEM_REACT_FROZENBROKEN_CRITICAL,
	//	FIGHT_PROP_ELEM_REACT_FROZENBROKEN_CRITICAL_HURT,
	//	FIGHT_PROP_ELEM_REACT_OVERGROW_CRITICAL,
	//	FIGHT_PROP_ELEM_REACT_OVERGROW_CRITICAL_HURT,
	//	FIGHT_PROP_ELEM_REACT_OVERGROW_FIRE_CRITICAL,
	//	FIGHT_PROP_ELEM_REACT_OVERGROW_FIRE_CRITICAL_HURT,
	//	FIGHT_PROP_ELEM_REACT_OVERGROW_ELECTRIC_CRITICAL,
	//	FIGHT_PROP_ELEM_REACT_OVERGROW_ELECTRIC_CRITICAL_HURT
	//};

	//void (*UpdateCombatProp_Original)(void* _this, FightPropType propType, float value, uint32_t state);
	//void UpdateCombatProp_Hook(void* _this, FightPropType propType, float value, uint32_t state) {
	//	// Проверяем, что _this относится к нашему персонажу (можно через campID или get_isAlive)

	//	// FightPropType для ATK обычно равен 4 или похожим значениям (нужно будет сдампить enum FightPropType)
	//	// Либо можно менять глобальный множитель урона (addHurtBase)

	//	if (propType == FightPropType::FIGHT_PROP_BASE_ATTACK || propType == FightPropType::FIGHT_PROP_CUR_ATTACK) {
	//		value = value * 100.0f; // Увеличиваем атаку в 100 раз
	//	}

	//	// Для Крита (FIGHT_PROP_CRITICAL)
	//	if (propType == FightPropType::FIGHT_PROP_CRITICAL) {
	//		value = 1.0f; // 100% шанс крита
	//	}
	//	// Для Крит урона (FIGHT_PROP_CRITICAL_HURT)
	//	if (propType == FightPropType::FIGHT_PROP_CRITICAL_HURT) {
	//		value = value * 50.0f; // Огромный крит урон
	//	}

	//	UpdateCombatProp_Original(_this, propType, value, state);
	//}

	void (*LCBaseCombat_FireBeingHitEvent)(Il2CppObject* _this, uint32_t attackeeRuntimeID, Il2CppObject* attackResult);
	void hLCBaseCombat_FireBeingHitEvent(Il2CppObject* _this, uint32_t attackeeRuntimeID, Il2CppObject* attackResult) {

		Log("0x10:%s\n", (*(Il2CppString**)((uintptr_t)attackResult + 0x10))->ToCStr());
		Log("0x18:%s\n", (*(Il2CppString**)((uintptr_t)attackResult + 0x18))->ToCStr());
		Log("0x20:%s\n", (*(Il2CppString**)((uintptr_t)attackResult + 0x20))->ToCStr());
		Log("0x48:%s\n", (*(Il2CppString**)((uintptr_t)attackResult + 0x48))->ToCStr());
		Log("0x60:%s\n", (*(Il2CppString**)((uintptr_t)attackResult + 0x60))->ToCStr());


		Log("0x120:%f\n", *(float*)((uintptr_t)attackResult + 0x120));
		Log("0x13C:%f\n", *(float*)((uintptr_t)attackResult + 0x13C));
		Log("0x140:%f\n", *(float*)((uintptr_t)attackResult + 0x140)); //
		Log("0x158:%f\n", *(float*)((uintptr_t)attackResult + 0x158));
		Log("0x1A8:%f\n", *(float*)((uintptr_t)attackResult + 0x1A8)); // bulletFlyTime

		Log("0x244:%f\n", *(float*)((uintptr_t)attackResult + 0x244));
		Log("0x248:%f\n", *(float*)((uintptr_t)attackResult + 0x248));
		Log("0x250:%f\n", *(float*)((uintptr_t)attackResult + 0x250));
		Log("0x258:%f\n", *(float*)((uintptr_t)attackResult + 0x258));
		Log("0x280:%f\n", *(float*)((uintptr_t)attackResult + 0x280)); // 


		*(float*)((uintptr_t)attackResult + 0x140) = 100.f;
		*(float*)((uintptr_t)attackResult + 0x280) = 100.f;


		LCBaseCombat_FireBeingHitEvent(_this, attackeeRuntimeID, attackResult);

		*(float*)((uintptr_t)attackResult + 0x140) = 100.f;
		*(float*)((uintptr_t)attackResult + 0x280) = 100.f;
	}


	// Сигнатура (проверь количество аргументов в дампе, обычно MethodInfo идет в конце)
	//void (*ResolveAttackResultByAttacker)(void* attackerEntity, void* attackeeEntity, void* attackResult, void* methodInfo);

	//	public static void [a-z]{11}\([a-z]{11} [a-z]{11}, [a-z]{11} [a-z]{11}, DCDKGOKNGIK [a-z]{11}\)

	//void hResolveAttackResultByAttacker(void* attackerEntity, void* attackeeEntity, void* attackResult, void* methodInfo) {
	//	// 1. Даем игре сначала рассчитать оригинальный урон

	//	if (attackResult) {
	//		*(float*)((uintptr_t)attackResult + 0x140) = 300.f;
	//		*(float*)((uintptr_t)attackResult + 0x280) = 300.f;

	//		Log("Attack Resolved: Damage modified!");
	//	}
	//	ResolveAttackResultByAttacker(attackerEntity, attackeeEntity, attackResult, methodInfo);
	//	if (attackResult) {
	//		*(float*)((uintptr_t)attackResult + 0x140) = 300.f;
	//		*(float*)((uintptr_t)attackResult + 0x280) = 300.f;

	//		Log("Attack Resolved: Damage modified!");
	//	}
	//}

	//	public static void [a-z]{11}\(PAGADEGKDNA [a-z]{11}, PAGADEGKDNA [a-z]{11}, DCDKGOKNGIK [a-z]{11}\)
	void (*CalcAttackResult)(void* a1, void* a2, void* attackResult, void* attackerEntity, void* attackeeEntity);
	void hCalcAttackResult(void* a1, void* a2, void* attackResult, void* attackerEntity, void* attackeeEntity) {
		CalcAttackResult(a1, a2, attackResult, attackerEntity, attackeeEntity);

		*(float*)((uintptr_t)attackResult + 0x140) = 300.f;
		*(float*)((uintptr_t)attackResult + 0x280) = 300.f;

		Log("Attack Resolved: Damage modified!");
	}

	void KillAura::OnInit() {
		/*MH_CreateHook((LPVOID)(Mem::Signature(
			"56 48 83 EC ? 0F 29 74 24 ? 48 89 CE 80 3D ? ? ? ? 00 75 ? F3 0F 10 76 ? E8 ? ? ? ? F3 0F 58 C6 F3 0F 11 46 ? E8 ? ? ? ? 0F 57 C0").Scan()),
			(LPVOID)hLevelSyncCombatPlugin_TickFlushTimeAcc, (LPVOID*)&LevelSyncCombatPlugin_TickFlushTimeAcc);*/

			/*MH_CreateHook((LPVOID)(g_game_base + 0x80E9AD0),
				(LPVOID)hAttackResult_CreateAttackResult, (LPVOID*)&AttackResult_CreateAttackResult);*/

				/*MH_CreateHook((LPVOID)(g_game_base + 0xE092D10),
					(LPVOID)hLCBaseCombat_FireBeingHitEvent, (LPVOID*)&LCBaseCombat_FireBeingHitEvent);*/

					/*MH_CreateHook((LPVOID)(g_game_base + 0x957F150),
						(LPVOID)hResolveAttackResultByAttacker, (LPVOID*)&ResolveAttackResultByAttacker);*/

						/*MH_CreateHook((LPVOID)(g_game_base + 0x9579B90),
								(LPVOID)hCalcAttackResult, (LPVOID*)&CalcAttackResult);*/

						/*MH_CreateHook((LPVOID)(g_game_base + 0xE0948C0),
							(LPVOID)UpdateCombatProp_Hook, (LPVOID*)&UpdateCombatProp_Original);*/


							//MH_CreateHook((LPVOID)(Mem::Signature("41 57 41 56 41 54 56 57 55 53 48 83 EC ? 45 89 CF 4D 89 C6 89 D5 48 89 CE").Scan()),
								//(LPVOID)hLevelSyncCombatPlugin_RequestSceneEntityMoveReq, (LPVOID*)&LevelSyncCombatPlugin_RequestSceneEntityMoveReq);
	}

	/*	void KillAura::OnUpdate()
		{
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

					Il2CppObject* logic_component_manager = *(Il2CppObject**)((uintptr_t)entity + 0x200); // 0x110
					if (!logic_component_manager) return;
					Log("[KillAura] logic_component_manager: %p\n", logic_component_manager);

					//Il2CppObject* lc_base_combat = ComponentManager_GetComponent(logic_component_manager, Il2CppString::FromCStr("BCAPHHFEMJK"));
					//if (!lc_base_combat) return;
					//Log("[KillAura] lc_base_combat: %p\n", lc_base_combat);

					//LCBaseCombat_ChangeHP(lc_base_combat, 0.f);
					//LCBaseCombat_ChangeHP1(lc_base_combat, 0.f);
					//LCBaseCombat_UpdateCombatProp(lc_base_combat, 2, 0, 1);
					//Log("[KillAura] done\n");


					Il2CppObject* obj = ComponentManager_GetComponent(logic_component_manager, Il2CppString::FromCStr("IGMMANECCMN"));
					if (!obj) return;
					Log("[KillAura] obj: %p\n", obj);

					///((void(*)(Il2CppObject * _this, int32_t type, float value, int32_t state))(g_game_base + 0x6E9B320))(obj, 2, 0, 1);
					//((void(*)(Il2CppObject * _this, float value))(g_game_base + 0x6E9B380))(obj, 0);
					//((void(*)(Il2CppObject * _this,  float value))(g_game_base + 0x6E9AEB0))(obj, 0);

					//((void(*)(Il2CppObject * _this, uint32_t killer, int32_t dieType))(g_game_base + 0x6E98B40))(obj, MoleMole::EntityManager::Instance()->GetAvatar()->GetRuntimeID(), 3);

					void* AttackResult = ((void* (*)(int32_t a, float b))(g_game_base + 0x80E9470))(1, 5.f);
					Il2CppClass* attackResultClass = Il2Cpp::Class::FromName("MoleMole", "AttackResult");
					Il2CppObject* o = il2cpp_object_new(attackResultClass);
					Log("[KillAura] done\n");
				}
			}
		}*/
}