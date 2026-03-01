#pragma once

#include "../internal_include.h"

namespace Protobuf
{
	template<typename T>
	struct RepeatedMessageField : Il2CppObject {
		Unity::List<T>* values;
		bool isInPool;
		int32_t count;
	};

	template<typename T>
	struct RepeatedPrimitiveField : Il2CppObject {
		Unity::Array<T>* array;
		int32_t count;
	};
}

namespace Proto
{
	struct AvatarInfo {
		//char _pad0[0x18];

		//void* some_pointer1;   // 0x18
		//void* some_pointer2;   // 0x20
		//void* some_pointer3;   // 0x28
		//void* some_pointer4;   // 0x30
		//void* some_pointer5;   // 0x38
		//void* some_pointer6;   // 0x40
		//void* some_pointer7;   // 0x48
		//void* some_pointer8;   // 0x50
		//void* some_pointer9;   // 0x58
		//void* some_pointer10;  // 0x60
		//void* some_pointer11;  // 0x68
		//void* some_pointer12;  // 0x70
		//void* some_pointer13;  // 0x78
		//void* some_pointer14;  // 0x80
		//void* some_pointer15;  // 0x88
		//void* some_pointer16;  // 0x90
		//void* some_pointer17;  // 0x98
		//void* some_pointer18;  // 0xA0

		//uint32_t some_uint1;   // 0xA8
		//uint32_t some_uint2;   // 0xAC
		//uint32_t some_uint3;   // 0xB0
		//uint32_t some_uint4;   // 0xB4
		//uint32_t some_uint5;   // 0xB8
		//uint32_t some_uint6;   // 0xBC

		//uint64_t guid; // 0xC0

		//uint32_t flycloak_id;   // 0xC8
		//bool     some_bool1;   // 0xCC

		//uint32_t config_id;   // 0xD0
		//uint32_t some_uint9;   // 0xD4
		//uint32_t costume_id;  // 0xD8
		//int32_t  some_int1;    // 0xDC
		//uint32_t some_uint11;  // 0xE0
		//uint32_t some_uint12;  // 0xE4
		//uint32_t some_uint13;  // 0xE8
		//uint32_t some_uint14;  // 0xEC

		uint32_t* ConfigID();
		uint32_t* CostumeID();
		uint32_t* FlycloakID();
	};

	struct AvatarDataNotify {
		//char _[0x18];
		//RepeatedPrimitiveField<uint64_t>* some_list1; // 0x18
		//RepeatedPrimitiveField<uint32_t>* ownedCostumeList_; // 0x20
		//void* NBCKLBCDFDH; // 0x28
		//RepeatedMessageField<AvatarInfo*>* avatarList_; // 0x30
		//void* NOGFCBAFIAK; // 0x38
		//RepeatedPrimitiveField<uint32_t>* some_list3; // 0x40
		//RepeatedPrimitiveField<uint32_t>* some_list4; // 0x48
		//RepeatedPrimitiveField<uint32_t>* some_list5; // 0x50
		//uint64_t chooseAvatarGuid_; // 0x58
		//uint32_t curAvatarTeamId_; // 0x60

		Protobuf::RepeatedMessageField<AvatarInfo*>* GetAvatarList();
	};

	struct SceneEntityInfo {
		//char _pad0[0x18];

		//void* some_pointer1;   // 0x18
		//Il2CppObject* entity;    // 0x20
		//void* some_pointer2;   // 0x28
		//Il2CppString* some_string1;    // 0x30

		//void* some_pointer3;           // 0x38
		//void* some_pointer4;           // 0x40
		//void* some_pointer5;           // 0x48
		//void* some_pointer6;           // 0x50
		//void* some_pointer7;           // 0x58
		//void* some_pointer8;           // 0x60
		//void* some_pointer9;           // 0x68
		//void* some_pointer10;          // 0x70
		//void* some_pointer11;          // 0x78
		//void* some_pointer12;          // 0x80

		//uint32_t some_uint1;           // 0x88
		//uint32_t some_uint2;           // 0x8C
		//uint32_t some_uint3;           // 0x90
		//uint32_t some_uint4;           // 0x94
		//int32_t  some_int1;            // 0x98
		//uint32_t some_uint5;           // 0x9C
		//int32_t  some_int2;            // 0xA0

		Il2CppObject* Entity();
	};

	struct SceneAvatarInfo {
		//char _pad0[0x18];

		//void* some_pointer1;    // 0x18
		//void* some_pointer2;    // 0x20
		//void* some_pointer3;    // 0x28
		//void* some_pointer4;    // 0x30
		//void* some_pointer5;    // 0x38
		//void* some_pointer6;    // 0x40
		//void* some_pointer7;    // 0x48
		//void* some_pointer8;    // 0x50
		//void* some_pointer9;    // 0x58
		//void* some_pointer10;   // 0x60
		//void* some_pointer11;   // 0x68
		//void* some_pointer12;   // 0x70
		//void* some_pointer13;   // 0x78
		//void* some_pointer14;   // 0x80
		//void* some_pointer15;   // 0x88
		//void* some_pointer16;   // 0x90

		//uint32_t config_id;    // 0x98
		//uint32_t flycloak_id;    // 0x9C
		//uint32_t some_uint3;    // 0xA0
		//uint32_t some_uint4;    // 0xA4
		//uint32_t some_uint5;    // 0xA8
		//uint32_t some_uint6;    // 0xAC
		//uint32_t some_uint7;    // 0xB0
		//uint32_t some_uint8;    // 0xB4
		//uint32_t some_uint9;    // 0xB8
		//uint32_t some_uint10;   // 0xBC

		//uint64_t some_uint64_1; // 0xC0

		//uint32_t some_uint11;  // 0xC8
		//uint32_t costume_id;  // 0xCC

		uint32_t* ConfigID();
		uint32_t* CostumeID();
		uint32_t* FlycloakID();
	};

	enum class MotionState {
		MotionNone,
		MotionReset,
		MotionStandby,
		MotionStandbyMove,
		MotionWalk,
		MotionRun,
		MotionDash,
		MotionClimb,
		MotionClimbJump,
		MotionStandbyToClimb,
		MotionFight,
		MotionJump,
		MotionDrop,
		MotionFly,
		MotionSwimMove,
		MotionSwimIdle,
		MotionSwimDash,
		MotionSwimJump,
		MotionSlip,
		MotionGoUpstairs,
		MotionFallOnGround,
		MotionJumpUpWallForStandby,
		MotionJumpOffWall,
		MotionPoweredFly,
		MotionLadderIdle,
		MotionLadderMove,
		MotionLadderSlip,
		MotionStandbyToLadder,
		MotionLadderToStandby,
		MotionDangerStandby,
		MotionDangerStandbyMove,
		MotionDangerWalk,
		MotionDangerRun,
		MotionDangerDash,
		MotionCrouchIdle,
		MotionCrouchMove,
		MotionCrouchRoll,
		MotionNotify,
		MotionLandSpeed,
		MotionMoveFailAck,
		MotionWaterfall,
		MotionDashBeforeShake,
		MotionSitIdle,
		MotionForceSetPos,
		MotionQuestForceDrag,
		MotionFollowRoute,
		MotionSkiffBoarding,
		MotionSkiffNormal,
		MotionSkiffDash,
		MotionSkiffPoweredDash,
		MotionDestroyVehicle,
		MotionFlyIdle,
		MotionFlySlow,
		MotionFlyFast,
		MotionAimMove,
		MotionAirCompensation,
		MotionSorushNormal,
		MotionRollerCoaster,
		MotionDiveIdle,
		MotionDiveMove,
		MotionDiveDash,
		MotionDiveDolphine,
		MotionDebug,
		MotionOceanCurrent,
		MotionDiveSwimMove,
		MotionDiveSwimIdle,
		MotionDiveSwimDash,
		MotionArcLight,
		MotionArcLightSafe,
		MotionVehicleStandby,
		MotionVehicleRun,
		MotionVehicleDash,
		MotionVehicleClimb,
		MotionVehicleClimbJump,
		MotionVehicleStandbyToClimb,
		MotionVehicleFight,
		MotionVehicleJump,
		MotionVehicleDrop,
		MotionVehicleFly,
		MotionVehicleSwimMove,
		MotionVehicleSwimIdle,
		MotionVehicleSwimDash,
		MotionVehicleSlip,
		MotionVehicleGoUpstairs,
		MotionVehicleFallOnGround,
		MotionVehicleJumpOffWall,
		MotionVehiclePoweredFly,
		MotionVehicleDangerStandby,
		MotionVehicleDangerRun,
		MotionVehicleDangerDash,
		MotionVehicleNotify,
		MotionVehicleLandSpeed,
		MotionVehicleDashBeforeShake,
		MotionVehicleQuestForceDrag,
		MotionVehicleFollowRoute,
		MotionVehicleFlyIdle,
		MotionVehicleFlySlow,
		MotionVehicleFlyFast,
		MotionVehicleAirCompensation,
		MotionVehicleArcLight,
		MotionVehicleArcLightSafe,
		MotionVehicleDangerSwimMove,
		MotionVehicleDangerSwimIdle,
		MotionVehicleDangerSwimDash,
		MotionFollowCurveRoute,
		MotionVehicleFollowCurveRoute,
		MotionNatsaurusNormal,
		MotionNatsaurusEntering,
		MotionMaglev,
		MotionMaglevSafe,
		MotionNum
	};

	struct MotionInfo {
		/*char _[0x18];
		ProtoVector* pos1;
		ProtoVector* pos2;
		ProtoVector* pos3;
		void* unk1;
		ProtoVector* pos4;*/

		MotionState& GetMotionState();
	};
}
