#pragma once
//#include "memory/offset_db.h"

namespace version_constants
{
	namespace beebyte
	{
		constexpr const char* entity_manager_class = "IOOEEBGFAIN"; // private Queue<Dictionary<UInt32,Int32>>
		constexpr const char* ui_manager_class = "DGGLGNIIFJE"; // private Dictionary<Int32,List<Notify>>
		constexpr const char* player_module_class = "FFKCIEJHLCI"; // private static MonoUIWaterMask
		constexpr const char* map_module_class = "PJEKOIAKMJM"; // private QuestEnterDungeonRequest
		constexpr const char* map_manager_class = "CEJAJIPNDNA"; // private Dictionary<UInt32,Tuple<String,String>>
		constexpr const char* loading_manager_class = "INPMGDILOOP"; // private Void [A-Z]{11}\(UInt32 [A-Z]{11}, IList<[A-Z]{11}> [A-Z]{11}\);
		constexpr const char* item_module_class = "AKELJJJLBCH"; // public Void [A-Z]{11}\(UInt32 [A-Z]{11}, List<UInt32> [A-Z]{11}, Boolean [A-Z]{11}\);
		constexpr const char* network_manager_class = "EBNFEFJIBAK"; // public ConfigChannel 
		constexpr const char* talk_dialog_context_class = "PGLHFCFGJEM"; // private MonoTalkDialog 
		
		//constexpr const char* avatar_entity_class = "OBEOOPGNGLK"; // public Void [A-Z]{11}\(Dictionary<Int32,List<[A-Z]{11}>> [A-Z]{11}, Boolean [A-Z]{11}\)
	}

	/*
	namespace offsets
	{
		//constexpr int ui_camera = 0x50; // "private Dictionary<Int32,List<Notify>> ", then "private Camera"
		OFFSET(ui_camera, "48 8B 7E ? 48 85 FF 74 ? 48 83 7F ? 00 74 ? 80 BE ? ? ? ? 00 74 ? 48 8B 8E")
		OFFSET(mono_in_level_map_page, "48 8B 8E ? ? ? ? 48 85 C9 0F 84 ? ? ? ? 48 89 DA E8 ? ? ? ? 48 89 F1")
		//constexpr int mono_in_level_map_page = 0x288; // "private MonoInLevelMapPage "
		//constexpr int map_view_rect = 0x50C; // "private MonoMapBanner ", then "private Rect "
		OFFSET(map_view_rect, "48 89 45 ? B9 ? ? ? ? ? ? ? 89 4D")
		OFFSET(entity_type, "83 BE ? ? ? ? ? 75 ? 48 8B 15 ? ? ? ? E8 ? ? ? ? 48 85 C0")
		OFFSET(runtime_id, "C7 87 ? ? ? ? 00 00 00 00 48 8B 87 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? FF 40 ? 44 8B 40 ? C7 40 ? 00 00 00 00 45 85 C0 7E ? 48 8B 48 ? 31 D2 E8 ? ? ? ? 48 8B 05")
		OFFSET(scene_entity_info_entity, "49 89 5C 24 ? 48 85 DB B8 ? ? ? ? E9 ? ? ? ? 3D ? ? ? ? 0F 84 ? ? ? ? 3D ? ? ? ? 0F 84")
		OFFSET(AvatarDataNotify, "49 89 5C 24 ? 48 85 DB B8 ? ? ? ? E9 ? ? ? ? 3D ? ? ? ? 0F 84 ? ? ? ? 3D ? ? ? ? 0F 84")
		
		//constexpr int runtime_id = 0x474; // C7 87 ? ? ? ? 00 00 00 00 48 8B 87 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? FF 40 ? 44 8B 40 ? C7 40 ? 00 00 00 00 45 85 C0 7E ? 48 8B 48 ? 31 D2 E8 ? ? ? ? 48 8B 05
		//constexpr int entity_type = 0x470; // 83 BE ? ? ? ? ? 75 ? 48 8B 15 ? ? ? ? E8 ? ? ? ? 48 85 C0
	}
	*/
}