#pragma once
#include "feature.h"

#include <vector>
#include <string>
#include <set>
#include <map>

namespace features
{
	struct Teleport {
		std::string name;
		Unity::Vector3 position;
		std::string description;
		// Full relative path including filename
		std::string relative_path;

		Teleport() : name(""), position({ 0, 0, 0 }), description(""), relative_path("") {}
		Teleport(const std::string& _name, Unity::Vector3 _pos, const std::string& _desc, const std::string& _rel_path = "")
			: name(_name), position(_pos), description(_desc), relative_path(_rel_path) {}
	};

	// Tree node for hierarchical folder structure
	struct CategoryNode {
		std::string name;
		std::vector<size_t> teleport_indices;
		std::map<std::string, CategoryNode> children;
		CategoryNode* parent = nullptr;
	};

	class CustomTeleports : public Feature {
	private:
		std::vector<Teleport> teleports;
		std::vector<std::string> teleport_files;
		std::set<size_t> checked_indices;
		CategoryNode root_category;
		std::vector<std::string> expanded_categories;

		// Auto teleport state
		bool auto_teleport_running = false;
		float auto_teleport_delay = 2.0f;
		int64_t next_teleport_time = 0;
		std::vector<size_t> auto_teleport_queue;
		size_t auto_teleport_current = 0;

		// Deferred deletion
		int deferred_delete_index = -1;

		void LoadTeleportsFromDisk();
		void SaveTeleport(const Teleport& teleport);
		std::string GetTeleportsDir() const;
		bool ValidateTeleportName(const std::string& name) const;
		void BuildAutoTeleportQueue();
		void ExecuteAutoTeleport();
		void BuildCategoryTree();
		void DrawCategoryTree(CategoryNode& node, const std::string& node_path);
		void ProcessDeferredDeletion();

	public:
		void DrawUI() override;
		void OnInit() override;
		void OnUpdate() override;

		void TeleportToPosition(const Unity::Vector3& position);
		void ReloadTeleports();
	};
}
