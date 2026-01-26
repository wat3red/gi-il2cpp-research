#pragma once

#include "../internal_include.h"

namespace MoleMole
{
	struct PlayerModule {
		static PlayerModule* Instance();
		uint32_t GetCurSceneID();
	};

	struct LoadingManager {
		static LoadingManager* Instance();
	};

	struct ActorUtils {
		static Unity::Vector3 GetAvatarPos();
		static void SetAvatarPos(const Unity::Vector3& position);
		static void SyncEntityPos(MoleMole::BaseEntity* entity, int32_t state, uint32_t mainQuestId);
	};

	struct ILuaActor {};

	struct BaseActor {
		ILuaActor* GetLuaActor();
	};

	struct ActorManager {
		static ActorManager* Instance();
		BaseActor* GetGlobalActor();
		BaseActor* GetActor(MoleMole::BaseEntity* entity);
	};

	struct TransmitRequest {
		BaseActor* actor; // 0x0
		uint32_t sceneId; // 0x8
		Unity::Vector3 targetPos; // 0xC
		Unity::Vector3 targetEuler; // 0x18
		Unity::Array<Il2CppString*>* textMapId; // 0x28
		Unity::Array<uint32_t>* dialogId; // 0x30
		bool useTextMapId; // 0x38
		Il2CppString* prefabPath; // 0x40
		Unity::Array<Il2CppString*>* picPaths; // 0x48
		Unity::Array<Il2CppString*>* picPathsGirl; // 0x50
		float picAutoNextInterval; // 0x58
		bool canSkipPic; // 0x5C
		bool isPicLoopPlay; // 0x5D
		float textShowTime; // 0x60
		struct System_Action_1_95* transFinishCallback; // 0x68
		struct System_Action_1_95* transPreCallback; // 0x70
		struct MoleMole_CurtainTask* task; // 0x78
		uint32_t questId; // 0x80
		uint32_t pointId; // 0x84
		uint32_t customTemplateId; // 0x88
		bool showBlackScreen; // 0x8C
		bool useWhiteScreen; // 0x8D
		bool disableTPAudio; // 0x8E
		Il2CppString* overrideTPAudioEvtName; // 0x90
		Il2CppString* loadCtxTag; // 0x98
		float loadCtxCloseDelay; // 0xA0
		bool interruptInteract; // 0xA4
		bool dontResetPlayerFSM; // 0xA5
		uint32_t targetArcCurveId; // 0xA8
		int32_t targetArcPointId; // 0xAC
		float targetArcInitialSpeed; // 0xB0
		float targetArcProgress; // 0xB4
		bool isKeepVehicle; // 0xB8
	};
}
