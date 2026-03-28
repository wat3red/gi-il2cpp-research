#pragma once
//#include "memory/offset_db.h"

namespace version_constants
{
	namespace beebyte {
		constexpr const char* entity_manager_class = "IOOEEBGFAIN"; // private Queue<Dictionary<UInt32,Int32>>
		constexpr const char* ui_manager_class = "DGGLGNIIFJE"; // private Dictionary<Int32,List<Notify>>
		constexpr const char* player_module_class = "FFKCIEJHLCI"; // private static MonoUIWaterMask
		constexpr const char* map_module_class = "PJEKOIAKMJM"; // private QuestEnterDungeonRequest
		constexpr const char* map_manager_class = "CEJAJIPNDNA"; // private Dictionary<UInt32,Tuple<String,String>>
		constexpr const char* loading_manager_class = "INPMGDILOOP"; // private Void [A-Z]{11}\(UInt32 [A-Z]{11}, IList<[A-Z]{11}> [A-Z]{11}\);
		constexpr const char* item_module_class = "AKELJJJLBCH"; // public Void [A-Z]{11}\(UInt32 [A-Z]{11}, List<UInt32> [A-Z]{11}, Boolean [A-Z]{11}\);
		constexpr const char* network_manager_class = "EBNFEFJIBAK"; // public ConfigChannel 
		constexpr const char* talk_dialog_context_class = "PGLHFCFGJEM"; // private MonoTalkDialog 
		constexpr const char* mark_manager_class = "CLFOHLGDFAL"; // private Dictionary<UInt32,SceneScriptData> 
		// private List<KeyValuePair<UInt32,Int32>> ComponentManager GOIEMHMONNA
		//constexpr const char* base_entity_class = "HCJGEEJOFPB"; // public static List<RuntimePlatform
		//constexpr const char* avatar_entity_class = "OBEOOPGNGLK"; // public Void [A-Z]{11}\(Dictionary<Int32,List<[A-Z]{11}>> [A-Z]{11}, Boolean [A-Z]{11}\)
	}

	namespace offsets {
		
	}
}