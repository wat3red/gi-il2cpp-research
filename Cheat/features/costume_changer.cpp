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
	void(*PlayerModule_OnAvatarDataNotify)(void* _this, Proto::AvatarDataNotify* notify);
	void hPlayerModule_OnAvatarDataNotify(void* _this, Proto::AvatarDataNotify* notify) {
		Log("[AvatarNotify] _this=%p notify=%p\n", _this, notify);

		Unity::List<Proto::AvatarInfo*>* list = notify->GetAvatarList()->values;

		for (size_t i = 0; i < list->size; i++) {
			Proto::AvatarInfo* avatar = list->array->items[i];

			Log("[%zu] val=%p\n", i, avatar);

			Config::CostumeMapping* mapping = config.costume_changer.GetMapping(*avatar->ConfigID());
			if (mapping) {
				if (mapping->costume_id != 0) {
					*avatar->CostumeID() = mapping->costume_id;
				}
				if (mapping->flycloak_id != 0) {
					*avatar->FlycloakID() = mapping->flycloak_id;
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
	void (*HandleAuthorityAvatarAppear)(void* _this, Proto::SceneEntityInfo* entity, int32_t type, uint32_t infoParam, uint32_t costumeID);
	void hHandleAuthorityAvatarAppear(void* _this, Proto::SceneEntityInfo* entity, int32_t type, uint32_t infoParam, uint32_t costumeID) {
		Log("[HandleAuthorityAvatarAppear] entity=%p, costumeID=%u, infoParam=%u\n", entity, costumeID, infoParam);

		static bool initialized = false;
		if (!initialized) {
			CollectItems();
			initialized = true;
		}

		auto scene_avatar = (Proto::SceneAvatarInfo*)entity->Entity();
		Config::CostumeMapping* mapping = config.costume_changer.GetMapping(*scene_avatar->ConfigID());
		if (mapping) {
			if (mapping->costume_id != 0) {
				*scene_avatar->CostumeID() = mapping->costume_id;
			}
			if (mapping->flycloak_id != 0) {
				*scene_avatar->FlycloakID() = mapping->flycloak_id;
			}
		}

		HandleAuthorityAvatarAppear(_this, entity, type, infoParam, costumeID);
	}

	void CostumeChanger::OnInit() {
		config.costume_changer.LoadMappings();

		MH_CreateHook(
			Mem::Signature("E8 ? ? ? ? 90 48 83 C4 ? C3 48 89 C1 4C 89 C2").ScanXref(),
			hHandleAuthorityAvatarAppear,
			(LPVOID*)&HandleAuthorityAvatarAppear
		);

		/*MH_CreateHook(
			(LPVOID)(g_game_base + 0xE7FD230),
			hPlayerModule_OnSceneTeamUpdateNotify,
			(LPVOID*)&PlayerModule_OnSceneTeamUpdateNotify
		);*/
		MH_CreateHook(
			Mem::Signature("55 41 57 41 56 41 55 41 54 56 57 53 48 81 EC ? ? ? ? 48 8D AC 24 ? ? ? ? 48 C7 85 ? ? ? ? ? ? ? ? 48 89 D6 48 89 CF 0F 57 C0").ScanXref(),
			hPlayerModule_OnAvatarDataNotify,
			(LPVOID*)&PlayerModule_OnAvatarDataNotify
		);

		MH_CreateHook(
			Mem::Signature("E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 8B 4C 24 ? E8").ScanXref(),
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
			//((void(*)(MoleMole::PlayerModule*, int32_t))(g_game_base + 0xE7B9290))(player_module, 1); // TryClientReconnect
			//((void(*)(MoleMole::PlayerModule*))(g_game_base + 0xE7DE970))(player_module); // NeedReloadScene
			Il2CppObject* network_manager = MoleMole::SingletonManager::GetSingletonInstance(version_constants::beebyte::network_manager_class);
			NetworkManager_HandleEnetLoginUnfinished(network_manager);
			reload = false;
		}
	}
}