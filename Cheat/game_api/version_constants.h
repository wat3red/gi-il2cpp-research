namespace version_constants
{
	namespace beebyte
	{
		constexpr const char* entity_manager_class = "GHGEBKKDDKE"; // private Queue<Dictionary<UInt32,Int32>>
		constexpr const char* ui_manager_class = "HPDJOMNOHPH"; // private Dictionary<Int32,List<Notify>>
		constexpr const char* player_module_class = "MHDKIGGLCHD"; // private static MonoUIWaterMask
		constexpr const char* map_module_class = "BBFIIGJPGMP"; // private QuestEnterDungeonRequest
		constexpr const char* map_manager_class = "FDAMOCLBAHC"; // private Dictionary<UInt32,Tuple<String,String>>
		constexpr const char* loading_manager_class = "JAMJFDLKMIC"; // private Void [A-Z]{11}\(UInt32 [A-Z]{11}, IList<[A-Z]{11}> [A-Z]{11}\);
		//constexpr const char* avatar_entity_class = "OBEOOPGNGLK"; // public Void [A-Z]{11}\(Dictionary<Int32,List<[A-Z]{11}>> [A-Z]{11}, Boolean [A-Z]{11}\)
		//public OBEOOPGNGLK .*\(\)
	}

	namespace offsets
	{
		constexpr int ui_camera = 0x50; // "private Dictionary<Int32,List<Notify>> ", then "private Camera"
		constexpr int mono_in_level_map_page = 0x288; // "private MonoInLevelMapPage "
		constexpr int map_view_rect = 0x50C; // 0x5B0 "private MonoMapBanner ", then "private Rect "

	}
}