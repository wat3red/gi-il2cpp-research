#include "custom_teleports.h"

#include <fstream>
#include <filesystem>
#include <map>
#include <sstream>
#include <chrono>
#include <shlwapi.h>
#include <game_api/game/singleton.h>
#include <external/json.hpp>

#pragma comment(lib, "shlwapi.lib")

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace features
{
	std::string CustomTeleports::GetTeleportsDir() const {
		char* app_data_ptr = nullptr;
		size_t len = 0;
		_dupenv_s(&app_data_ptr, &len, "APPDATA");
		std::string dir;
		if (app_data_ptr) {
			dir = std::string(app_data_ptr) + "\\GICheat\\teleports";
			free(app_data_ptr);
		}
		else {
			dir = "C:\\Users\\Default\\AppData\\Roaming\\GICheat\\teleports";
		}
		return dir;
	}

	void CustomTeleports::LoadTeleportsFromDisk() {
		teleports.clear();
		teleport_files.clear();
		root_category = CategoryNode();
		root_category.name = "Teleports";

		std::string dir = GetTeleportsDir();
		if (!fs::exists(dir)) {
			fs::create_directories(dir);
			return;
		}

		try {
			for (const auto& entry : fs::recursive_directory_iterator(dir)) {
				if (entry.is_directory()) continue;
				if (entry.path().extension() == ".json") {
					std::ifstream file(entry.path());
					if (!file.is_open()) {
						Log("Failed to open %s\n", entry.path().string().c_str());
						continue;
					}

					std::string content((std::istreambuf_iterator<char>(file)),
						std::istreambuf_iterator<char>());
					file.close();

					try {
						json j = json::parse(content);

						std::string name = j.value("name", entry.path().stem().string());
						Unity::Vector3 pos = {
							j.value("position", json::array({0.f, 0.f, 0.f}))[0].get<float>(),
							j.value("position", json::array({0.f, 0.f, 0.f}))[1].get<float>(),
							j.value("position", json::array({0.f, 0.f, 0.f}))[2].get<float>()
						};
						std::string desc = j.value("description", std::string(""));

						// Get full relative path
						std::string relative_path = fs::relative(entry.path(), dir).string();

						teleports.push_back(Teleport(name, pos, desc, relative_path));
						teleport_files.push_back(entry.path().string());

						Log("Loaded teleport: %s from %s\n", name.c_str(), relative_path.c_str());
					}
					catch (const json::exception& e) {
						Log("JSON parse error in %s: %s\n", entry.path().string().c_str(), e.what());
						continue;
					}
				}
			}
		}
		catch (const fs::filesystem_error& e) {
			Log("Filesystem error: %s\n", e.what());
		}

		BuildCategoryTree();
	}

	void CustomTeleports::BuildCategoryTree() {
		root_category.teleport_indices.clear();
		root_category.children.clear();

		for (size_t i = 0; i < teleports.size(); i++) {
			const auto& tp = teleports[i];
			std::string path = tp.relative_path;

			// Remove filename to get folder path
			size_t last_slash = path.find_last_of("\\");
			if (last_slash == std::string::npos) {
				// File in root
				root_category.teleport_indices.push_back(i);
				continue;
			}

			std::string folder_path = path.substr(0, last_slash);

			// Navigate/create tree structure
			CategoryNode* current = &root_category;
			std::stringstream ss(folder_path);
			std::string folder;

			while (std::getline(ss, folder, '\\')) {
				if (folder.empty()) continue;

				if (current->children.find(folder) == current->children.end()) {
					current->children[folder] = CategoryNode();
					current->children[folder].name = folder;
					current->children[folder].parent = current;
				}
				current = &current->children[folder];
			}

			current->teleport_indices.push_back(i);
		}
	}

	bool CustomTeleports::ValidateTeleportName(const std::string& name) const {
		if (name.empty()) return false;

		// Check for invalid filename characters
		const std::string invalid = "\\/:*?\"<>|";
		if (name.find_first_of(invalid) != std::string::npos) {
			return false;
		}

		// Check for duplicates
		for (const auto& tp : teleports) {
			if (tp.name == name) {
				return false;
			}
		}

		return true;
	}

	void CustomTeleports::SaveTeleport(const Teleport& teleport) {
		std::string dir = GetTeleportsDir();
		if (!fs::exists(dir)) {
			fs::create_directories(dir);
		}

		try {
			json j;
			j["name"] = teleport.name;
			j["position"] = { teleport.position.x, teleport.position.y, teleport.position.z };
			j["description"] = teleport.description;

			// Save to root by default or to specified path if provided
			std::string filepath = dir + "\\" + teleport.name + ".json";
			if (!teleport.relative_path.empty()) {
				fs::path save_path = fs::path(dir) / fs::path(teleport.relative_path);
				if (!save_path.parent_path().empty() && save_path.parent_path() != fs::path(dir)) {
					fs::create_directories(save_path.parent_path());
				}
				filepath = save_path.string();
			}

			std::ofstream file(filepath);
			if (!file.is_open()) {
				Log("Failed to create teleport file: %s\n", filepath.c_str());
				return;
			}

			file << j.dump(4);
			file.close();

			Log("Saved teleport: %s\n", teleport.name.c_str());
		}
		catch (const std::exception& e) {
			Log("Error saving teleport: %s\n", e.what());
		}
	}

	void CustomTeleports::TeleportToPosition(const Unity::Vector3& position) {
		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		if (!entity_manager) {
			Log("EntityManager not available\n");
			return;
		}

		MoleMole::BaseEntity* avatar = entity_manager->GetAvatar();
		if (!avatar) {
			Log("Avatar not available\n");
			return;
		}

		MoleMole::ActorUtils::SetAvatarPos(position);
		MoleMole::ActorUtils::SyncEntityPos(entity_manager->GetAvatar(), 0, 0);

		Log("Teleported to: %f, %f, %f\n", position.x, position.y, position.z);
	}

	void CustomTeleports::ReloadTeleports() {
		LoadTeleportsFromDisk();
	}

	void CustomTeleports::OnInit() {
		LoadTeleportsFromDisk();
	}

	void CustomTeleports::OnUpdate() {
		// Process deferred deletion after UI is done rendering
		if (deferred_delete_index >= 0) {
			ProcessDeferredDeletion();
		}

		if (auto_teleport_running) {
			ExecuteAutoTeleport();
		}
	}

	void CustomTeleports::BuildAutoTeleportQueue() {
		auto_teleport_queue.clear();
		auto_teleport_current = 0;

		// Build queue from checked indices (sorted)
		std::vector<size_t> sorted_indices(checked_indices.begin(), checked_indices.end());
		std::sort(sorted_indices.begin(), sorted_indices.end());

		auto_teleport_queue = sorted_indices;
		next_teleport_time = 0;
	}

	void CustomTeleports::ExecuteAutoTeleport() {
		if (auto_teleport_queue.empty()) {
			auto_teleport_running = false;
			return;
		}

		auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count();

		if (current_time >= next_teleport_time) {
			if (auto_teleport_current < auto_teleport_queue.size()) {
				size_t tp_idx = auto_teleport_queue[auto_teleport_current];
				if (tp_idx < teleports.size()) {
					TeleportToPosition(teleports[tp_idx].position);
					Log("Auto TP: %s (%zu/%zu)\n", teleports[tp_idx].name.c_str(),
						auto_teleport_current + 1, auto_teleport_queue.size());
				}

				auto_teleport_current++;
				next_teleport_time = current_time + (int64_t)(auto_teleport_delay * 1000.0f);
			}
			else {
				auto_teleport_running = false;
				Log("Auto TP completed\n");
			}
		}
	}

	void CustomTeleports::ProcessDeferredDeletion() {
		if (deferred_delete_index < 0 || deferred_delete_index >= (int)teleports.size()) {
			return;
		}

		const auto& tp = teleports[deferred_delete_index];
		std::string full_path = GetTeleportsDir() + "\\" + tp.relative_path;

		try {
			if (fs::exists(full_path)) {
				fs::remove(full_path);
				Log("Deleted teleport: %s from %s\n", tp.name.c_str(), full_path.c_str());
			}
		}
		catch (const fs::filesystem_error& e) {
			Log("Error deleting file: %s\n", e.what());
		}

		deferred_delete_index = -1;
		LoadTeleportsFromDisk();
		checked_indices.clear();
	}

	void CustomTeleports::DrawCategoryTree(CategoryNode& node, const std::string& node_path) {
		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		Unity::Vector3 avatar_pos = entity_manager ? entity_manager->GetAvatar()->GetRelativePosition() : Unity::Vector3{ 0, 0, 0 };

		// Draw teleports in current node
		for (size_t idx : node.teleport_indices) {
			if (idx >= teleports.size()) continue;

			const auto& tp = teleports[idx];
			float distance = avatar_pos.Distance(tp.position);

			bool is_checked = checked_indices.find(idx) != checked_indices.end();
			std::string tp_id = "##tp_" + std::to_string(idx);

			// Only show checkbox if auto teleport option is enabled
			if (config.custom_teleports.auto_teleport) {
				if (ImGui::Checkbox(tp_id.c_str(), &is_checked)) {
					if (is_checked)
						checked_indices.insert(idx);
					else
						checked_indices.erase(idx);
				}

				ImGui::SameLine();
			}

			// Teleport name and info
			ImGui::BeginGroup();
			{
				ImGui::Text("%s", tp.name.c_str());
				ImGui::TextDisabled("Pos: %.1f, %.1f, %.1f | Dist: %.1f m", tp.position.x, tp.position.y, tp.position.z, distance);
				if (!tp.description.empty()) {
					ImGui::TextDisabled("Desc: %s", tp.description.c_str());
				}
			}
			ImGui::EndGroup();

			ImGui::SameLine(0, 20);

			// Action buttons
			std::string tp_btn = "TP##" + std::to_string(idx);
			std::string del_btn = "Del##" + std::to_string(idx);

			if (ImGui::Button(tp_btn.c_str(), ImVec2(40, 0))) {
				TeleportToPosition(tp.position);
			}

			ImGui::SameLine();
			if (ImGui::Button(del_btn.c_str(), ImVec2(40, 0))) {
				// Mark for deferred deletion - will be processed in OnUpdate after UI is done
				deferred_delete_index = idx;
			}

			ImGui::Separator();
		}

		// Draw child folders as tree nodes
		for (auto& [folder_name, child_node] : node.children) {
			std::string child_path = node_path.empty() ? folder_name : node_path + "/" + folder_name;
			std::string tree_label = folder_name + "##" + child_path;

			// Check if all teleports in this category (recursively) are selected
			std::function<bool(CategoryNode&)> all_checked = [&](CategoryNode& n) -> bool {
				for (size_t idx : n.teleport_indices) {
					if (checked_indices.find(idx) == checked_indices.end()) {
						return false;
					}
				}
				for (auto& [_, child] : n.children) {
					if (!all_checked(child)) {
						return false;
					}
				}
				return true;
			};

			// Create tree node with category checkbox
			bool category_checked = all_checked(child_node);
			std::string cat_checkbox_id = "##cat_select_" + child_path;

			ImGui::BeginGroup();
			{
				ImGui::PushID(child_path.c_str());

				bool category_checked = all_checked(child_node);

				// Чекбокс
				if (config.custom_teleports.auto_teleport) {
					if (ImGui::Checkbox("##select_all", &category_checked)) {
						std::function<void(CategoryNode&, bool)> toggle_category =
							[&](CategoryNode& n, bool check) {
							for (size_t idx : n.teleport_indices) {
								if (check)
									checked_indices.insert(idx);
								else
									checked_indices.erase(idx);
							}
							for (auto& [_, child] : n.children) {
								toggle_category(child, check);
							}
							};

						toggle_category(child_node, category_checked);
					}

					ImGui::SameLine();
				}

				// Tree node
				bool open = ImGui::TreeNodeEx(folder_name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow);

				if (open) {
					DrawCategoryTree(child_node, child_path);
					ImGui::TreePop();
				}

				ImGui::PopID();
			}
			ImGui::EndGroup();
		}
	}

	void CustomTeleports::DrawUI() {
		static char name_buffer[256] = { 0 };
		static char description_buffer[256] = { 0 };
		static char json_buffer[4096] = { 0 };

		ImGui::Text("Custom Teleports (%zu loaded)", teleports.size());
		ImGui::Separator();

		if (ImGui::BeginTabBar("CustomTeleportsTab")) {
			if (ImGui::BeginTabItem("Add Teleport")) {
				ImGui::InputText("Name##teleport", name_buffer, sizeof(name_buffer));
				ImGui::InputText("Description##teleport", description_buffer, sizeof(description_buffer));

				if (ImGui::Button("Add Current Position")) {
					std::string name_str(name_buffer);
					std::string desc_str(description_buffer);

					if (!ValidateTeleportName(name_str)) {
						Log("Invalid teleport name: %s\n", name_str.c_str());
					}
					else {
						MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
						if (entity_manager) {
							Unity::Vector3 pos = entity_manager->GetAvatar()->GetAbsolutePosition();
							Teleport tp(name_str, pos, desc_str);
							teleports.push_back(tp);
							SaveTeleport(tp);
							memset(name_buffer, 0, sizeof(name_buffer));
							memset(description_buffer, 0, sizeof(description_buffer));
						}
					}
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Import JSON")) {
				ImGui::InputTextMultiline("JSON Input##teleport", json_buffer, sizeof(json_buffer), ImVec2(0, 100), ImGuiInputTextFlags_AllowTabInput);

				if (ImGui::Button("Import")) {
					std::string json_str(json_buffer);
					try {
						json j = json::parse(json_str);

						std::string name = j.value("name", std::string("Imported"));
						if (!ValidateTeleportName(name)) {
							Log("Invalid teleport name or duplicate\n");
						}
						else {
							Unity::Vector3 pos = {
								j.value("position", json::array({0.f, 0.f, 0.f}))[0].get<float>(),
								j.value("position", json::array({0.f, 0.f, 0.f}))[1].get<float>(),
								j.value("position", json::array({0.f, 0.f, 0.f}))[2].get<float>()
							};
							std::string desc = j.value("description", std::string(""));

							Teleport tp(name, pos, desc);
							teleports.push_back(tp);
							SaveTeleport(tp);
							memset(json_buffer, 0, sizeof(json_buffer));
							Log("Imported teleport: %s\n", name.c_str());
						}
					}
					catch (const json::exception& e) {
						Log("JSON parse error: %s\n", e.what());
					}
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Manage")) {
				if (ImGui::Button("Open Folder")) {
					std::string dir = GetTeleportsDir();
					if (!fs::exists(dir)) {
						fs::create_directories(dir);
					}
					ShellExecuteA(NULL, "open", dir.c_str(), NULL, NULL, SW_SHOW);
				}

				ImGui::SameLine();
				if (ImGui::Button("Reload")) {
					LoadTeleportsFromDisk();
					checked_indices.clear();
					Log("Reloaded teleports\n");
				}

				ImGui::Separator();

				// Teleport tree list
				ImGui::BeginChild("TeleportList", ImVec2(0, 300), true);
				{
					if (ImGui::TreeNodeEx("Teleports", ImGuiTreeNodeFlags_DefaultOpen)) {
						DrawCategoryTree(root_category, "");
						ImGui::TreePop();
					}
				}
				ImGui::EndChild();

				ImGui::Separator();

				// Auto teleport controls
				ImGui::BeginGroup();
				{
					ImGuiEx::Checkbox("Auto Teleport", config.custom_teleports.auto_teleport);
					if (config.custom_teleports.auto_teleport) {
						ImGui::Indent();
						ImGui::SetNextItemWidth(150);
						ImGui::DragFloat("Delay (s)##auto_tp", &auto_teleport_delay, 0.1f, 0.5f, 30.0f);

						if (ImGui::Button("Start Auto TP")) {
							if (!checked_indices.empty()) {
								BuildAutoTeleportQueue();
								auto_teleport_running = true;
								next_teleport_time = 0;
							}
						}

						ImGui::SameLine();
						if (ImGui::Button("Stop Auto TP")) {
							auto_teleport_running = false;
							auto_teleport_queue.clear();
							auto_teleport_current = 0;
						}

						if (auto_teleport_running && !auto_teleport_queue.empty()) {
							ImGui::SameLine();
							ImGui::TextColored(ImVec4(0, 1, 0, 1), "Running: %zu/%zu", auto_teleport_current, auto_teleport_queue.size());
						}

						ImGui::SameLine(0, 20);
						ImGui::Text("Selected: %zu", checked_indices.size());
						ImGui::Unindent();
					}
				}
				ImGui::EndGroup();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
}
