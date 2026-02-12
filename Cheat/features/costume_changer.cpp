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

#include <unordered_map>

namespace features
{
	bool reload = false;
	std::unordered_map<uint32_t, std::string> all_avatars;
	std::unordered_map<uint32_t, std::string> all_costumes;
	std::unordered_map<uint32_t, std::string> all_flycloaks;

	// UI State
	static uint32_t selected_avatar_id = 0;
	static uint32_t selected_costume_id = 0;
	static uint32_t selected_flycloak_id = 0;
	static bool show_add_dialog = false;
	static int delete_index = -1;

	void CostumeChanger::DrawUI() {
		ImGuiEx::Checkbox("Enable costume changer", config.costume_changer.enabled);

		if (config.costume_changer.enabled) {
			ImGui::Indent();

			if (ImGui::Button("Reload Data")) {
				reload = true;
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// Add new mapping section
			if (ImGui::CollapsingHeader("Add Avatar Mapping", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Indent();

				// Avatar selection
				if (ImGui::BeginCombo("##avatar_select",
					selected_avatar_id == 0 ? "Select Avatar..." :
					(all_avatars.count(selected_avatar_id) ? all_avatars[selected_avatar_id].c_str() : "Unknown"))) {
					for (const auto& [id, name] : all_avatars) {
						bool is_selected = (selected_avatar_id == id);
						if (ImGui::Selectable(name.c_str(), is_selected)) {
							selected_avatar_id = id;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				// Costume selection
				if (ImGui::BeginCombo("##costume_select",
					selected_costume_id == 0 ? "Select Costume..." :
					(all_costumes.count(selected_costume_id) ? all_costumes[selected_costume_id].c_str() : "Unknown"))) {
					if (ImGui::Selectable("None##costume_none", selected_costume_id == 0)) {
						selected_costume_id = 0;
					}
					for (const auto& [id, name] : all_costumes) {
						bool is_selected = (selected_costume_id == id);
						if (ImGui::Selectable(name.c_str(), is_selected)) {
							selected_costume_id = id;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				// Flycloak selection
				if (ImGui::BeginCombo("##flycloak_select",
					selected_flycloak_id == 0 ? "Select Flycloak..." :
					(all_flycloaks.count(selected_flycloak_id) ? all_flycloaks[selected_flycloak_id].c_str() : "Unknown"))) {
					if (ImGui::Selectable("None##flycloak_none", selected_flycloak_id == 0)) {
						selected_flycloak_id = 0;
					}
					for (const auto& [id, name] : all_flycloaks) {
						bool is_selected = (selected_flycloak_id == id);
						if (ImGui::Selectable(name.c_str(), is_selected)) {
							selected_flycloak_id = id;
						}
						if (is_selected) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				ImGui::Spacing();

				if (ImGui::Button("Add Mapping", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
					if (selected_avatar_id != 0) {
						config.costume_changer.SaveMapping(selected_avatar_id, selected_costume_id, selected_flycloak_id);
						selected_avatar_id = 0;
						selected_costume_id = 0;
						selected_flycloak_id = 0;
					}
				}

				ImGui::Unindent();
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// Existing mappings section
			if (ImGui::CollapsingHeader("Mappings", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Indent();

				if (config.costume_changer.mappings.empty()) {
					ImGui::TextDisabled("No mappings added yet");
				}
				else {
					ImGui::BeginChild("mappings_child", ImVec2(0, 200), true);

					for (int i = 0; i < config.costume_changer.mappings.size(); i++) {
						auto& mapping = config.costume_changer.mappings[i];
						std::string avatar_name = all_avatars.count(mapping.avatar_id) ?
							all_avatars[mapping.avatar_id] : ("Avatar #" + std::to_string(mapping.avatar_id));
						std::string costume_name = mapping.costume_id == 0 ? "None" :
							(all_costumes.count(mapping.costume_id) ?
								all_costumes[mapping.costume_id] : ("Costume #" + std::to_string(mapping.costume_id)));
						std::string flycloak_name = mapping.flycloak_id == 0 ? "None" :
							(all_flycloaks.count(mapping.flycloak_id) ?
								all_flycloaks[mapping.flycloak_id] : ("Flycloak #" + std::to_string(mapping.flycloak_id)));

						ImGui::PushID(i);
						ImGui::BeginGroup();

						ImGui::Text("%s", avatar_name.c_str());
						ImGui::Indent();
						ImGui::TextColored(ImVec4(0.5f, 1.f, 0.5f, 1.f), "Costume: %s", costume_name.c_str());
						ImGui::TextColored(ImVec4(1.f, 0.84f, 0.f, 1.f), "Flycloak: %s", flycloak_name.c_str());
						ImGui::Unindent();

						if (ImGui::Button("Remove", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
							delete_index = i;
						}

						ImGui::EndGroup();
						ImGui::PopID();
						ImGui::Separator();
					}

					ImGui::EndChild();
				}

				if (delete_index >= 0 && delete_index < config.costume_changer.mappings.size()) {
					uint32_t avatar_to_remove = config.costume_changer.mappings[delete_index].avatar_id;
					config.costume_changer.mappings.erase(config.costume_changer.mappings.begin() + delete_index);
					delete_index = -1;
				}

				ImGui::Unindent();
			}

			ImGui::Unindent();
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

		uint64_t guid; // 0xC0

		uint32_t flycloak_id;   // 0xC8
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

	struct AvatarCostumeExcelConfig : Il2CppObject {
		Il2CppString* some_string1; // 0x10
		Il2CppString* json_name; // 0x18
		Il2CppString* some_string3; // 0x20
	};

	struct AvatarFlycloakExcelConfig : Il2CppObject {
		Il2CppString* some_string1; // 0x10
		Il2CppString* json_name; // 0x18
		Il2CppString* some_string3; // 0x20
	};

	struct AvatarExcelConfig {};

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

		for (size_t i = 0; i < notify->avatarList_->values->size; i++) {
			AvatarInfo* value = notify->avatarList_->values->array->items[i];

			Log("[%zu] val=%p\n", i, value);

			auto mapping = config.costume_changer.GetMapping(value->config_id);
			if (mapping) {
				if (mapping->costume_id != 0) {
					value->costume_id = mapping->costume_id;
				}
				if (mapping->flycloak_id != 0) {
					value->flycloak_id = mapping->flycloak_id;
				}
			}
		}

		PlayerModule_OnAvatarDataNotify(_this, notify);
	}

	bool (*CanChangeAvatarEntity)(MoleMole::BaseEntity* entity, unsigned int configId, int failType, bool ignoreStateLayer, bool isSpecialChange, bool ignoreCollision);
	bool hCanChangeAvatarEntity(MoleMole::BaseEntity* entity, unsigned int configId, int failType, bool ignoreStateLayer, bool isSpecialChange, bool ignoreCollision) {
		//Log("hCanChangeAvatarEntity, name=%s, configId=%u\n", entity->GetName()->ToCStr(), configId);
		//return CanChangeAvatarEntity(entity, configId, failType, ignoreStateLayer, isSpecialChange, ignoreCollision);
		return true;
	}

	struct AvatarChangeCostumeRsp {
		char _[0x18];
		uint32_t costume_id;
		int32_t ret_code;
		uint64_t some_ulong;
	};

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
		uint32_t flycloak_id;    // 0x9C
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

	void CollectItems() {
		//((void(*)())(g_game_base + 0x8B61E50))(); // prepare
		auto costumes = (((Unity::Dictionary<uint32_t, AvatarCostumeExcelConfig*> *(*)())(g_game_base + 0x8B61E40))());
		if (costumes) {
			auto costumes_vector = costumes->to_vector();
			//Log("Available costumes (%zu total):\n", costumes_vector.size());
			for (size_t i = 0; i < costumes_vector.size(); i++) {
				if (costumes_vector[i].first % 100 != 1 && costumes_vector[i].first % 100 != 2)
					continue;
				//Log("  Costume ID: %u - %s\n",
					//costumes_vector[i].first,
					//costumes_vector[i].second->json_name->ToCStr());
				all_costumes.insert({ costumes_vector[i].first, costumes_vector[i].second->json_name->ToCStr() });
			}
		}

		//((void(*)())(g_game_base + 0xE805890))(); // prepare
		auto flycloaks = (((Unity::Dictionary<uint32_t, AvatarFlycloakExcelConfig*> *(*)())(g_game_base + 0xE805CA0))());
		if (flycloaks) {
			auto flycloaks_vector = flycloaks->to_vector();
			//Log("Available flycloaks_vector  (%zu total):\n", flycloaks_vector.size());
			for (size_t i = 0; i < flycloaks_vector.size(); i++) {
				//Log("  Flycloak ID: %u - %s\n",
					//flycloaks_vector[i].first,
					//flycloaks_vector[i].second->json_name->ToCStr());
				all_flycloaks.insert({ flycloaks_vector[i].first, flycloaks_vector[i].second->json_name->ToCStr() });
			}
		}

		//((void(*)())(g_game_base + 0xD4CD3A0))(); // prepare
		auto avatars = (((Unity::Dictionary<uint32_t, AvatarExcelConfig*> *(*)())(g_game_base + 0xD4CD380))());
		if (avatars) {
			auto avatars_vector = avatars->to_vector();
			//Log("Available avatars (%zu total):\n", avatars_vector.size());
			for (size_t i = 0; i < avatars_vector.size(); i++) {
				//Log("  Avatar ID: %u - '%s'\n",
					//avatars_vector[i].first,
					//((Il2CppString * (*)(AvatarExcelConfig*))(g_game_base + 0xEC558E0))(avatars_vector[i].second)->ToCStr());
				all_avatars.insert({ avatars_vector[i].first, ((Il2CppString * (*)(AvatarExcelConfig*))(g_game_base + 0xEC558E0))(avatars_vector[i].second)->ToCStr() });
			}
		}
	}

	// 	private void [A-Z]{11}\([A-Z]{11} [A-Z]{11}, LBBDEIFADJM [A-Z]{11}, uint32 [A-Z]{11}, uint32 [A-Z]{11}\)
	// 	private void [A-Z]{11}\([A-Z]{11} [A-Z]{11}, JNJPFIJANIF [A-Z]{11}, uint32 [A-Z]{11}, uint32 [A-Z]{11}\) // 6.2
	void (*HandleAuthorityAvatarAppear)(void* _this, SceneEntityInfo* entity, int32_t type, uint32_t infoParam, uint32_t costumeID);
	void hHandleAuthorityAvatarAppear(void* _this, SceneEntityInfo* entity, int32_t type, uint32_t infoParam, uint32_t costumeID) {
		Log("[HandleAuthorityAvatarAppear] entity=%p, costumeID=%u, infoParam=%u\n", entity, costumeID, infoParam);

		static bool initialized = false;
		if (!initialized) {
			CollectItems();
			initialized = true;
		}

		auto scene_avatar = (SceneAvatarInfo*)entity->entity;
		auto mapping = config.costume_changer.GetMapping(scene_avatar->config_id);
		if (mapping) {
			if (mapping->costume_id != 0) {
				scene_avatar->costume_id = mapping->costume_id;
			}
			if (mapping->flycloak_id != 0) {
				scene_avatar->flycloak_id = mapping->flycloak_id;
			}
		}

		HandleAuthorityAvatarAppear(_this, entity, type, infoParam, costumeID);
	}

	void CostumeChanger::OnInit() {
		config.costume_changer.LoadMappings();

		MH_CreateHook(
			(LPVOID)(g_game_base + 0xE7D7290),
			hHandleAuthorityAvatarAppear,
			(LPVOID*)&HandleAuthorityAvatarAppear
		);

		/*MH_CreateHook(
			(LPVOID)(g_game_base + 0xE7FD230),
			hPlayerModule_OnSceneTeamUpdateNotify,
			(LPVOID*)&PlayerModule_OnSceneTeamUpdateNotify
		);*/

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

		// 6.3
		// AppearancePageContext_OnCostumeSelect: 0x108F7440
		// private void [A-Z]{11}\(int32 [A-Z]{11}, boolean [A-Z]{11}, boolean [A-Z]{11}\)
	}

	void CostumeChanger::OnUpdate() {
		if (reload) {
			MoleMole::PlayerModule* player_module = MoleMole::PlayerModule::Instance();
			// TryClientReconnect
			((void(*)(MoleMole::PlayerModule*, int32_t))(g_game_base + 0xE7B9290))(player_module, 1);
			reload = false;
		}
	}
}