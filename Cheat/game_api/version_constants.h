#pragma once

// Obfuscated class names change every patch. Each entry below is anchored by a
// unique dumped type/field/method signature that still identifies the manager
// after a rename. Refresh the names from a fresh Dumper run; keep the anchors.
namespace version_constants
{
	namespace beebyte {
		// anchor: private Queue<Dictionary<UInt32,Int32>>
		constexpr const char* entity_manager_class = "IOOEEBGFAIN";
		// anchor: private Dictionary<Int32,List<Notify>>
		constexpr const char* ui_manager_class = "DGGLGNIIFJE";
		// anchor: private static MonoUIWaterMask
		constexpr const char* player_module_class = "FFKCIEJHLCI";
		// anchor: private QuestEnterDungeonRequest
		constexpr const char* map_module_class = "PJEKOIAKMJM";
		// anchor: private Dictionary<UInt32,Tuple<String,String>>
		constexpr const char* map_manager_class = "CEJAJIPNDNA";
		// anchor: private Void [A-Z]{11}\(UInt32 [A-Z]{11}, IList<[A-Z]{11}> [A-Z]{11}\);
		constexpr const char* loading_manager_class = "INPMGDILOOP";
		// anchor: public Void [A-Z]{11}\(UInt32 [A-Z]{11}, List<UInt32> [A-Z]{11}, Boolean [A-Z]{11}\);
		constexpr const char* item_module_class = "AKELJJJLBCH";
		// anchor: public ConfigChannel
		constexpr const char* network_manager_class = "EBNFEFJIBAK";
		// anchor: private MonoTalkDialog
		constexpr const char* talk_dialog_context_class = "PGLHFCFGJEM";
		// anchor: private Dictionary<UInt32,SceneScriptData>
		constexpr const char* mark_manager_class = "CLFOHLGDFAL";

		// Also seen while hunting ComponentManager:
		//   private List<KeyValuePair<UInt32,Int32>>  -> GOIEMHMONNA
		// Candidates parked during earlier patches:
		//   base_entity_class   "HCJGEEJOFPB"  // public static List<RuntimePlatform
		//   avatar_entity_class "OBEOOPGNGLK"  // public Void [A-Z]{11}\(Dictionary<Int32,List<[A-Z]{11}>> [A-Z]{11}, Boolean [A-Z]{11}\)
	}

	namespace offsets {
	}
}
