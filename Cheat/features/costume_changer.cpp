#define IMGUI_DEFINE_MATH_OPERATORS
#define _CRT_SECURE_NO_WARNINGS

#include "costume_changer.h"

#include <game_api/include.h>
#include <logger/logger.h>
#include <config/imgui_config.h>
#include <config/config.h>

#include <imgui/imgui.h>
#include <minhook/include/MinHook.h>
#include <imgui/imgui_internal.h>

namespace features
{
	bool reload = false;
	void CostumeChanger::DrawUI() {
		if (ImGui::Button("reload")) {
			reload = !reload;
		}
	}

	void CostumeChanger::DrawBackgroundUI() {}

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

	struct AvatarInfo {
		char _pad0[0x18];

		void* some_pointer1;   // 0x18
		void* some_pointer2;   // 0x20
		void* some_pointer3;   // 0x28
		void* some_pointer4;   // 0x30
		void* some_pointer5;   // 0x38
		void* some_pointer6;   // 0x40
		void* some_pointer7;   // 0x48
		void* some_pointer8;   // 0x50
		void* some_pointer9;   // 0x58
		void* some_pointer10;  // 0x60
		void* some_pointer11;  // 0x68
		void* some_pointer12;  // 0x70
		void* some_pointer13;  // 0x78
		void* some_pointer14;  // 0x80
		void* some_pointer15;  // 0x88
		void* some_pointer16;  // 0x90
		void* some_pointer17;  // 0x98
		void* some_pointer18;  // 0xA0

		uint32_t some_uint1;   // 0xA8
		uint32_t some_uint2;   // 0xAC
		uint32_t some_uint3;   // 0xB0
		uint32_t some_uint4;   // 0xB4
		uint32_t some_uint5;   // 0xB8
		uint32_t some_uint6;   // 0xBC

		uint64_t some_uint64_1; // 0xC0

		uint32_t some_uint7;   // 0xC8
		bool     some_bool1;   // 0xCC

		uint32_t config_id;   // 0xD0
		uint32_t some_uint9;   // 0xD4
		uint32_t costume_id;  // 0xD8
		int32_t  some_int1;    // 0xDC
		uint32_t some_uint11;  // 0xE0
		uint32_t some_uint12;  // 0xE4
		uint32_t some_uint13;  // 0xE8
		uint32_t some_uint14;  // 0xEC
	};

	void LogAvatarInfo(AvatarInfo* v) {
		if (!v) {
			Log("AvatarInfo = NULL\n");
			return;
		}

		/*Log("  some_pointer1   = %p\n", v->some_pointer1);
		Log("  some_pointer2   = %p\n", v->some_pointer2);
		Log("  some_pointer3   = %p\n", v->some_pointer3);
		Log("  some_pointer4   = %p\n", v->some_pointer4);
		Log("  some_pointer5   = %p\n", v->some_pointer5);
		Log("  some_pointer6   = %p\n", v->some_pointer6);
		Log("  some_pointer7   = %p\n", v->some_pointer7);
		Log("  some_pointer8   = %p\n", v->some_pointer8);
		Log("  some_pointer9   = %p\n", v->some_pointer9);
		Log("  some_pointer10  = %p\n", v->some_pointer10);
		Log("  some_pointer11  = %p\n", v->some_pointer11);
		Log("  some_pointer12  = %p\n", v->some_pointer12);
		Log("  some_pointer13  = %p\n", v->some_pointer13);
		Log("  some_pointer14  = %p\n", v->some_pointer14);
		Log("  some_pointer15  = %p\n", v->some_pointer15);
		Log("  some_pointer16  = %p\n", v->some_pointer16);
		Log("  some_pointer17  = %p\n", v->some_pointer17);
		Log("  some_pointer18  = %p\n", v->some_pointer18);*/

		/*Log("  some_uint1      = %u\n", v->some_uint1);
		Log("  some_uint2      = %u\n", v->some_uint2);
		Log("  some_uint3      = %u\n", v->some_uint3);
		Log("  some_uint4      = %u\n", v->some_uint4);
		Log("  some_uint5      = %u\n", v->some_uint5);
		Log("  some_uint6      = %u\n", v->some_uint6);*/

		Log("  some_uint64_1   = %llu\n", (unsigned long long)v->some_uint64_1);

		Log("  some_uint7      = %u\n", v->some_uint7);
		//Log("  some_bool1      = %d\n", v->some_bool1 ? 1 : 0);

		Log("  configId        = %u\n", v->config_id);

		//Log("  some_uint9      = %u\n", v->some_uint9);
		Log("  costume_id     = %u\n", v->costume_id);
		/*Log("  some_int1       = %d\n", v->some_int1);
		Log("  some_uint11     = %u\n", v->some_uint11);
		Log("  some_uint12     = %u\n", v->some_uint12);
		Log("  some_uint13     = %u\n", v->some_uint13);
		Log("  some_uint14     = %u\n", v->some_uint14);*/
	}




	struct AvatarDataNotify {
		char _[0x18];
		RepeatedPrimitiveField<uint64_t>* some_list1; // 0x18
		RepeatedPrimitiveField<uint32_t>* ownedCostumeList_; // 0x20
		void* NBCKLBCDFDH; // 0x28
		RepeatedMessageField<AvatarInfo*>* avatarList_; // 0x30
		void* NOGFCBAFIAK; // 0x38
		RepeatedPrimitiveField<uint32_t>* some_list3; // 0x40
		RepeatedPrimitiveField<uint32_t>* some_list4; // 0x48
		RepeatedPrimitiveField<uint32_t>* some_list5; // 0x50
		uint64_t chooseAvatarGuid_; // 0x58
		uint32_t curAvatarTeamId_; // 0x60
	};

	struct SimpleSafeUInt32 {
		uint32_t value;

		uint32_t get_Value() {
			return (((uint32_t(*)(SimpleSafeUInt32*))(g_game_base + 0x8EC70))(this));
		}
	};

	struct SimpleSafeFloat {
		uint32_t value;

		float get_Value() {
			return (((float(*)(SimpleSafeFloat*))(g_game_base + 0xBA940))(this));
		}
	};

	struct AvatarCostumeExcelConfig : Il2CppObject {
		Il2CppString* some_string1; // 0x10
		Il2CppString* json_name; // 0x18
		Il2CppString* some_string3; // 0x20
		uint64_t some_ulong0; // 0x28
		uint32_t some_uint1; // 0x30
		uint32_t some_uint2; // 0x34
		uint32_t some_uint3; // 0x38
		uint64_t some_ulong1; // 0x40
		uint64_t some_ulong2; // 0x48
		uint64_t some_ulong3; // 0x50
		uint64_t some_ulong4; // 0x58
		uint64_t some_ulong5; // 0x60
		uint64_t some_ulong6; // 0x68
		SimpleSafeUInt32 some_safeuint1; // 0x70
		SimpleSafeUInt32 some_safeuint2; // 0x74
		uint64_t some_ulong7; // 0x78
		uint64_t some_ulong8; // 0x80
		uint64_t some_ulong9; // 0x88
		SimpleSafeUInt32 some_safeuint3; // 0x90
		SimpleSafeUInt32 some_safeuint4; // 0x94
		SimpleSafeUInt32 some_safeuint5; // 0x98
		SimpleSafeUInt32 some_safeuint6; // 0x9C
		uint64_t some_ulong10; // 0xA0
		bool some_bool1; // 0xA8
		bool some_bool2; // 0xA9
		bool some_bool3; // 0xAA
		bool some_bool4; // 0xAB
		bool some_bool5; // 0xAC
	};

	struct AvatarExcelConfig {
		char _[0xC0];
		Unity::Array<int32_t>* ACFKHKHMKJL; // 0xC0
		Il2CppString* idk_empty; // 0xC8
		Unity::Array<SimpleSafeUInt32>* EPFADMHLAJP; // 0xD0
		Unity::Array<SimpleSafeUInt32>* OBOABMJFAEG; // 0xD8
		Il2CppString* _imageName; // 0xE0
		Il2CppString* _iconName; // 0xE8
		Unity::Array<SimpleSafeUInt32>* AJHBEFMJMFA; // 0xF0
		Il2CppString* _sideIconName; // 0xF8
		int32_t NDHGDFNPOMA; // 0x100
		SimpleSafeUInt32 LPJBOJIDOGM; // 0x104
		SimpleSafeUInt32 IGIGEFEHEGB; // 0x108
		SimpleSafeFloat FFEEGCJCBGB; // 0x10C
		uint64_t MNLCGMLCEOJ; // 0x110
		uint64_t LELIHKNAOLI; // 0x118
		uint64_t BKCLJKHLOHI; // 0x120
		uint64_t AEJAOKJELGB; // 0x128
		SimpleSafeUInt32 HAGKGEEAAIK; // 0x130
		int32_t DJHBFCEFEMH; // 0x134
		uint64_t NAMLGBCJDPN; // 0x138
		uint32_t MFKBHKOOCKE; // 0x140
		SimpleSafeUInt32 GFHKLHKMCPJ; // 0x144
		SimpleSafeFloat JCDGLGIEIOP; // 0x148
		int32_t NEDENPLBPOG; // 0x14C
		uint64_t OBGGFJKAOJC; // 0x150
		uint64_t BCKLNKFFIBN; // 0x158
		uint64_t KLONFAHGEOE; // 0x160
		SimpleSafeUInt32 OOMNOELEHIL; // 0x168
		SimpleSafeUInt32 OLHONFHJPJN; // 0x16C
		uint64_t NAKPAEBDJAC; // 0x170
		uint64_t FCONGEPGDOI; // 0x178
		uint64_t KKFPOIBHPEK; // 0x180
		uint64_t FDLBGDHIIOP; // 0x188
		uint64_t OHNKPJFCKIC; // 0x190
		uint64_t KBLLPKCJBCA; // 0x198
		int32_t LOOECLFAHPE; // 0x1A0
		int32_t GBGOEFNFDLJ; // 0x1A4
		uint64_t BMJJNGBPOCH; // 0x1A8
		uint64_t NIDCKFMJHLB; // 0x1B0
		uint64_t LNMIDNHMJFC; // 0x1B8
		int32_t CKANNPNKIPF; // 0x1C0
		bool BALJJCELJFD; // 0x1C4
		bool INIIGAGFDMG; // 0x1C5
		bool GAPNFMHMEJI; // 0x1C6
		SimpleSafeFloat MGPMHIOHGMK; // 0x1C8
		int32_t LMAACLELEMN; // 0x1CC
		SimpleSafeUInt32 NMLPBBNLLFA; // 0x1D0
		SimpleSafeFloat CBCFOLHOMDM; // 0x1D4
		int32_t POBGBOLAJFP; // 0x1D8
		uint32_t EEPHCBGPCAA; // 0x1DC
	};

	// Proto.SceneAvatarInfo : MMKPLKAMPOE
	// Proto.SceneTeamUpdateNotify : JFAKGAMNMHC
	// Proto.SceneTeamAvatar : EPMCGHOMOOL
	// Proto.AvatarChangeCostumeRsp : CJALBADBHKA
	// Proto.AvatarDataNotify : MBDKEBFNKOE
	// Proto.AvatarInfo : HKOGCHIIJHP
	// OnAvatarDataNotify : IKMOJIKGBFB
	// MoleMole.Config.AvatarExcelConfig : DMCPPFHDBHK
	// SimpleSafeUInt32 : JNBFFKMNBLO
	// MoleMole.PlayerModule : MHDKIGGLCHD
	void(*PlayerModule_OnAvatarDataNotify)(void* _this, AvatarDataNotify* notify);
	void hPlayerModule_OnAvatarDataNotify(void* _this, AvatarDataNotify* notify) {
		Log("[AvatarNotify] _this=%p notify=%p\n", _this, notify);
		Log("chooseAvatarGuid_=%lu, curAvatarTeamId_=%u\n", notify->chooseAvatarGuid_, notify->curAvatarTeamId_);

		((void(*)())(g_game_base + 0x8B61E50))(); // prepare
		auto costumes = (((Unity::Dictionary<uint32_t, AvatarCostumeExcelConfig*> *(*)())(g_game_base + 0x8B61E40))());
		if (costumes) {
			auto costumes_vector = costumes->to_vector();
			Log("Available costumes (%zu total):\n", costumes_vector.size());
			for (size_t i = 0; i < costumes_vector.size(); i++) {
				Log("  Costume ID: %u - %s\n",
					costumes_vector[i].first,
					costumes_vector[i].second->json_name->ToCStr());
			}
		}

		((void(*)())(g_game_base + 0xD4CD3A0))(); // prepare
		auto avatars = (((Unity::Dictionary<uint32_t, AvatarExcelConfig*> *(*)())(g_game_base + 0xD4CD380))());
		if (avatars) {
			auto avatars_vector = avatars->to_vector();
			Log("Available avatars (%zu total):\n", avatars_vector.size());
			for (size_t i = 0; i < avatars_vector.size(); i++) {
				Log("  Avatar ID: %u - '%s'\n",
					avatars_vector[i].first,
					((Il2CppString * (*)(AvatarExcelConfig*))(g_game_base + 0xEC558E0))(avatars_vector[i].second)->ToCStr());
			}
		}

		for (size_t i = 0; i < notify->avatarList_->values->size; i++) {
			AvatarInfo* value = notify->avatarList_->values->array->items[i];

			Log("[%zu] val=%p\n", i, value);
			LogAvatarInfo(value);

			if (value->config_id == 10000005) {
				Log("----------------- FOUND\n");
				value->costume_id = 200501;
			}
		}

		PlayerModule_OnAvatarDataNotify(_this, notify);
	}

	struct SceneTeamAvatar {
		char _pad0[0x18];

		void* some_pointer1;   // 0x18
		AvatarInfo* avatarInfo;       // 0x20

		void* some_pointer2;          // 0x28
		void* some_pointer3;          // 0x30
		void* some_pointer4;          // 0x38
		void* some_pointer5;          // 0x40
		void* some_pointer6;          // 0x48

		uint32_t some_uint1;          // 0x50
		uint32_t some_uint2;          // 0x54
		uint64_t some_uint64_1;       // 0x58
		uint32_t some_uint3;          // 0x60

		bool some_bool1;              // 0x64
		bool some_bool2;              // 0x65
		bool some_bool3;              // 0x66
		// padding 0x67

		uint32_t some_uint4;          // 0x68
		uint64_t some_uint64_2;       // 0x70
	};


	void LogSceneTeamAvatar(SceneTeamAvatar* v) {
		if (!v) {
			Log("SceneTeamAvatar = NULL\n");
			return;
		}

		Log("SceneTeamAvatar @ %p\n", v);

		Log("  some_pointer1   = %p\n", v->some_pointer1);
		Log("  avatarInfo     = %p\n", v->avatarInfo);

		Log("  some_pointer2  = %p\n", v->some_pointer2);
		Log("  some_pointer3  = %p\n", v->some_pointer3);
		Log("  some_pointer4  = %p\n", v->some_pointer4);
		Log("  some_pointer5  = %p\n", v->some_pointer5);
		Log("  some_pointer6  = %p\n", v->some_pointer6);

		Log("  some_uint1     = %u\n", v->some_uint1);
		Log("  some_uint2     = %u\n", v->some_uint2);
		Log("  some_uint64_1  = %llu\n", (unsigned long long)v->some_uint64_1);
		Log("  some_uint3     = %u\n", v->some_uint3);

		Log("  some_bool1     = %d\n", v->some_bool1 ? 1 : 0);
		Log("  some_bool2     = %d\n", v->some_bool2 ? 1 : 0);
		Log("  some_bool3     = %d\n", v->some_bool3 ? 1 : 0);

		Log("  some_uint4     = %u\n", v->some_uint4);
		Log("  some_uint64_2  = %llu\n", (unsigned long long)v->some_uint64_2);

		if (v->avatarInfo) {
			Log("  -> AvatarInfo:\n");
			LogAvatarInfo(v->avatarInfo);
		}
	}


	static_assert(offsetof(SceneTeamAvatar, avatarInfo) == 0x20, "asd");


	struct SceneTeamUpdateNotify {
		char _[0x18];
		RepeatedMessageField<SceneTeamAvatar*>* sceneTeamAvatarList_;
		bool isInMp_;
	};

	void(*PlayerModule_OnSceneTeamUpdateNotify)(void* _this, SceneTeamUpdateNotify* notify);
	void hPlayerModule_OnSceneTeamUpdateNotify(void* _this, SceneTeamUpdateNotify* notify) {
		Log("[OnSceneTeamUpdateNotify] _this=%p notify=%p\n", _this, notify);

		Log("%zu total\n", notify->sceneTeamAvatarList_->values->size);
		for (size_t i = 0; i < notify->sceneTeamAvatarList_->values->size; i++) {
			LogSceneTeamAvatar(notify->sceneTeamAvatarList_->values->array->items[i]);

			AvatarInfo* value = notify->sceneTeamAvatarList_->values->array->items[i]->avatarInfo;
			Log("[%zu] val=%p\n", i, value);

			if (!value) continue;

			LogAvatarInfo(value);

			if (value->config_id == 10000005) {
				Log("----------------- FOUND\n");
				value->costume_id = 200501;
				Log("!!!!!!!!!!!!!!!!!!!!!!Updated costume in SceneTeamUpdate\n");

			}
		}
		PlayerModule_OnSceneTeamUpdateNotify(_this, notify);

	}

	/*void (*AppearancePageContext_OnCostumeSelect)(void* _this, int32_t index, bool isSetup, bool isJoypadInit);
	void hAppearancePageContext_OnCostumeSelect(void* _this, int32_t index, bool isSetup, bool isJoypadInit) {
		Log("hAppearancePageContext_OnCostumeSelect\n");
		AppearancePageContext_OnCostumeSelect(_this, index, isSetup, isJoypadInit);
	}*/

	bool (*CanChangeAvatarEntity)(MoleMole::BaseEntity* entity, unsigned int configId, int failType, bool ignoreStateLayer, bool isSpecialChange, bool ignoreCollision);
	bool hCanChangeAvatarEntity(MoleMole::BaseEntity* entity, unsigned int configId, int failType, bool ignoreStateLayer, bool isSpecialChange, bool ignoreCollision) {
		Log("hCanChangeAvatarEntity, name=%s, configId=%u\n", entity->GetName()->ToCStr(), configId);

		//return CanChangeAvatarEntity(entity, configId, failType, ignoreStateLayer, isSpecialChange, ignoreCollision);
		return true;
	}

	struct AvatarChangeCostumeRsp {
		char _[0x18];
		uint32_t costume_id;
		int32_t ret_code;
		uint64_t some_ulong;
	};

	void (*OnAvatarChangeCostumeRsp)(void* _this, AvatarChangeCostumeRsp* rsp);
	void hOnAvatarChangeCostumeRsp(void* _this, AvatarChangeCostumeRsp* rsp) {
		Log("hOnAvatarChangeCostumeRsp, costume_id=%u, ret_code=%d, some_ulong=%lu\n", rsp->costume_id, rsp->ret_code, rsp->some_ulong);
		//rsp->ret_code = 0;
		OnAvatarChangeCostumeRsp(_this, rsp);
	}

	struct SceneAvatarInfo {
		char _pad0[0x18];

		void* some_pointer1;    // 0x18
		void* some_pointer2;    // 0x20
		void* some_pointer3;    // 0x28
		void* some_pointer4;    // 0x30
		void* some_pointer5;    // 0x38
		void* some_pointer6;    // 0x40
		void* some_pointer7;    // 0x48
		void* some_pointer8;    // 0x50
		void* some_pointer9;    // 0x58
		void* some_pointer10;   // 0x60
		void* some_pointer11;   // 0x68
		void* some_pointer12;   // 0x70
		void* some_pointer13;   // 0x78
		void* some_pointer14;   // 0x80
		void* some_pointer15;   // 0x88
		void* some_pointer16;   // 0x90

		uint32_t config_id;    // 0x98
		uint32_t some_uint2;    // 0x9C
		uint32_t some_uint3;    // 0xA0
		uint32_t some_uint4;    // 0xA4
		uint32_t some_uint5;    // 0xA8
		uint32_t some_uint6;    // 0xAC
		uint32_t some_uint7;    // 0xB0
		uint32_t some_uint8;    // 0xB4
		uint32_t some_uint9;    // 0xB8
		uint32_t some_uint10;   // 0xBC

		uint64_t some_uint64_1; // 0xC0

		uint32_t some_uint11;  // 0xC8
		uint32_t costume_id;  // 0xCC
	};

	void LogSceneAvatarInfo(SceneAvatarInfo* v) {
		if (!v) {
			Log("SceneAvatarInfo = NULL\n");
			return;
		}

		Log("SceneAvatarInfo @ %p\n", v);

		Log("  some_pointer1   = %p\n", v->some_pointer1);
		Log("  some_pointer2   = %p\n", v->some_pointer2);
		Log("  some_pointer3   = %p\n", v->some_pointer3);
		Log("  some_pointer4   = %p\n", v->some_pointer4);
		Log("  some_pointer5   = %p\n", v->some_pointer5);
		Log("  some_pointer6   = %p\n", v->some_pointer6);
		Log("  some_pointer7   = %p\n", v->some_pointer7);
		Log("  some_pointer8   = %p\n", v->some_pointer8);
		Log("  some_pointer9   = %p\n", v->some_pointer9);
		Log("  some_pointer10  = %p\n", v->some_pointer10);
		Log("  some_pointer11  = %p\n", v->some_pointer11);
		Log("  some_pointer12  = %p\n", v->some_pointer12);
		Log("  some_pointer13  = %p\n", v->some_pointer13);
		Log("  some_pointer14  = %p\n", v->some_pointer14);
		Log("  some_pointer15  = %p\n", v->some_pointer15);
		Log("  some_pointer16  = %p\n", v->some_pointer16);

		Log("  config_id      = %u\n", v->config_id);
		Log("  some_uint2      = %u\n", v->some_uint2);
		Log("  some_uint3      = %u\n", v->some_uint3);
		Log("  some_uint4      = %u\n", v->some_uint4);
		Log("  some_uint5      = %u\n", v->some_uint5);
		Log("  some_uint6      = %u\n", v->some_uint6);
		Log("  some_uint7      = %u\n", v->some_uint7);
		Log("  some_uint8      = %u\n", v->some_uint8);
		Log("  some_uint9      = %u\n", v->some_uint9);
		Log("  some_uint10     = %u\n", v->some_uint10);

		Log("  some_uint64_1   = %llu\n", (unsigned long long)v->some_uint64_1);

		Log("  some_uint11     = %u\n", v->some_uint11);
		Log("  costume_id     = %u\n", v->costume_id);
	}


	struct SceneEntityInfo {
		char _pad0[0x18];

		void* some_pointer1;   // 0x18
		Il2CppObject* entity;    // 0x20
		void* some_pointer2;   // 0x28
		Il2CppString* some_string1;    // 0x30

		void* some_pointer3;           // 0x38
		void* some_pointer4;           // 0x40
		void* some_pointer5;           // 0x48
		void* some_pointer6;           // 0x50
		void* some_pointer7;           // 0x58
		void* some_pointer8;           // 0x60
		void* some_pointer9;           // 0x68
		void* some_pointer10;          // 0x70
		void* some_pointer11;          // 0x78
		void* some_pointer12;          // 0x80

		uint32_t some_uint1;           // 0x88
		uint32_t some_uint2;           // 0x8C
		uint32_t some_uint3;           // 0x90
		uint32_t some_uint4;           // 0x94
		int32_t  some_int1;            // 0x98
		uint32_t some_uint5;           // 0x9C
		int32_t  some_int2;            // 0xA0
	};

	void LogSceneEntityInfo(SceneEntityInfo* v) {
		if (!v) {
			Log("SceneEntityInfo = NULL\n");
			return;
		}

		Log("SceneEntityInfo @ %p\n", v);

		Log("  some_pointer1  = %p\n", v->some_pointer1);
		Log("  entity   = %p\n", v->entity);

		LogSceneAvatarInfo((SceneAvatarInfo*)v->entity);

		Log("  some_pointer2  = %p\n", v->some_pointer2);
		Log("  some_string1   = %p\n", v->some_string1);

		if (v->some_string1)
			Log("    string = %s\n", v->some_string1->ToCStr());

		Log("  some_pointer3  = %p\n", v->some_pointer3);
		Log("  some_pointer4  = %p\n", v->some_pointer4);
		Log("  some_pointer5  = %p\n", v->some_pointer5);
		Log("  some_pointer6  = %p\n", v->some_pointer6);
		Log("  some_pointer7  = %p\n", v->some_pointer7);
		Log("  some_pointer8  = %p\n", v->some_pointer8);
		Log("  some_pointer9  = %p\n", v->some_pointer9);
		Log("  some_pointer10 = %p\n", v->some_pointer10);
		Log("  some_pointer11 = %p\n", v->some_pointer11);
		Log("  some_pointer12 = %p\n", v->some_pointer12);

		Log("  some_uint1     = %u\n", v->some_uint1);
		Log("  some_uint2     = %u\n", v->some_uint2);
		Log("  some_uint3     = %u\n", v->some_uint3);
		Log("  some_uint4     = %u\n", v->some_uint4);
		Log("  some_int1      = %d\n", v->some_int1);
		Log("  some_uint5     = %u\n", v->some_uint5);
		Log("  some_int2      = %d\n", v->some_int2);
	}


	// 	private void [A-Z]{11}\([A-Z]{11} [A-Z]{11}, LBBDEIFADJM [A-Z]{11}, uint32 [A-Z]{11}, uint32 [A-Z]{11}\)
	// 	private void [A-Z]{11}\([A-Z]{11} [A-Z]{11}, JNJPFIJANIF [A-Z]{11}, uint32 [A-Z]{11}, uint32 [A-Z]{11}\) // 6.2
	
	void (*HandleAuthorityAvatarAppear)(void* _this, SceneEntityInfo* entity, int32_t type, uint32_t infoParam, uint32_t costumeID);
	void hHandleAuthorityAvatarAppear(void* _this, SceneEntityInfo* entity, int32_t type, uint32_t infoParam, uint32_t costumeID) {
		// Здесь можно подменить costumeID перед появлением

		Log("[HandleAuthorityAvatarAppear] entity=%p, costumeID=%u, infoParam=%u\n", entity, costumeID, infoParam);

		LogSceneEntityInfo(entity);
		if (((SceneAvatarInfo*)entity->entity)->config_id == 10000005) {
			((SceneAvatarInfo*)entity->entity)->costume_id = 200501;
		}

		/*Mem::Signature sig("49 89 5C 24 ? 48 85 DB B8 ? ? ? ? E9 ? ? ? ? 3D ? ? ? ? 0F 84 ? ? ? ? 3D ? ? ? ? 0F 84");
		uint8_t offset = *((uint8_t*)sig.Scan() + 4);*/

		HandleAuthorityAvatarAppear(_this, entity, type, infoParam, costumeID);
	}

	void CostumeChanger::OnInit() {
		MH_CreateHook(
			(LPVOID)(g_game_base + 0xE7D7290),
			hHandleAuthorityAvatarAppear,
			(LPVOID*)&HandleAuthorityAvatarAppear
		);

		MH_CreateHook(
			(LPVOID)(g_game_base + 0xE7FD230),
			hPlayerModule_OnSceneTeamUpdateNotify,
			(LPVOID*)&PlayerModule_OnSceneTeamUpdateNotify
		);

		MH_CreateHook(
			(LPVOID)(g_game_base + 0xE7CA9D0),
			hPlayerModule_OnAvatarDataNotify,
			(LPVOID*)&PlayerModule_OnAvatarDataNotify
		);

		MH_CreateHook(
			(LPVOID)(g_game_base + 0xFD7E180),
			hCanChangeAvatarEntity,
			(LPVOID*)&CanChangeAvatarEntity
		);

		/*MH_CreateHook(
			(LPVOID)(g_game_base + 0xE7C4200),
			hOnAvatarChangeCostumeRsp,
			(LPVOID*)&OnAvatarChangeCostumeRsp
		);*/

		// private void [A-Z]{11}\(int32 [A-Z]{11}, boolean [A-Z]{11}, boolean [A-Z]{11}\)
		/*MH_CreateHook(
			(LPVOID)(g_game_base + 0x108F7440),
			hAppearancePageContext_OnCostumeSelect,
			(LPVOID*)&AppearancePageContext_OnCostumeSelect
		);*/
	}

	void CostumeChanger::OnUpdate() {
		if (reload) {
			MoleMole::PlayerModule* player_module = MoleMole::PlayerModule::Instance();
			((void(*)(MoleMole::PlayerModule*, int32_t))(g_game_base + 0xE7B9290))(player_module, 1);
			reload = false;
		}
	}
}