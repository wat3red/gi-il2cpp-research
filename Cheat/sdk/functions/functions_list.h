//public virtual Void ClosePage();
RESOLVE_BY_OFFSET(void, BasePageContext_ClosePage, 0xE9D3FC0, (void* _this))

//public static Single CalcCurrentGroundHeight(Single x, Single z);
RESOLVE_BY_OFFSET(float, Miscs_CalcCurrentGroundHeight, 0xF4A9EB0, (float x, float z))

//public static Vector3d GetRelativePosition(Vector3d pos);
RESOLVE_BY_OFFSET(Unity::Vector3, WorldShiftManager_GetRelativePosition, 0x5E74CD0, (Unity::Vector3 pos))

//public static Vector3 GenWorldPos(Vector2 levelMapPos);
RESOLVE_BY_OFFSET(Unity::Vector3, Miscs_GenWorldPos, 0xF49E820, (Unity::Vector2 levelMapPos))

//public static Boolean ScreenPointToLocalPointInRectangle(RectTransform rect, Vector2 screenPoint, Camera cam, Vector2& localPoint);
RESOLVE_BY_OFFSET(bool, RectTransformUtility_ScreenPointToLocalPointInRectangle, 0x14FD4890, (void* rect, Unity::Vector2 screenPoint, Unity::Camera* cam, Unity::Vector2& localPoint))

//public Transform get_mapBackground();
RESOLVE_BY_OFFSET(Unity::Transform*, MonoInLevelMapPage_get_mapBackground, 0xC458930, (void* _this))
//public Rect get_mapRect(); // FLAGS: 0x886
RESOLVE_BY_OFFSET(MoleMole::Rect, MonoInLevelMapPage_get_mapRect, 0xC459090, (void* _this))

//public Object GetSingletonInstance(String typeName); 
RESOLVE_BY_OFFSET(Il2CppObject*, SingletonManager_GetSingletonInstance, 0x5CA0A30, (MoleMole::SingletonManager* _this, Unity::String* typeName))
// public static SingletonManager get_Instance(); 
RESOLVE_BY_OFFSET(MoleMole::SingletonManager*, SingletonManager_get_Instance, 0x5CA0B60, ())

//direct: 56 57 48 83 EC ? 48 89 CE 80 3D ? ? ? ? 00 48 8B 05 ? ? ? ? 0F 85 ? ? ? ? 48 8B 88 ? ? ? ? 48 85 C9 0F 84 ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C0
RESOLVE_BY_OFFSET(void, HDIKLPILBAC_CFDHCLDPCHI, 0xBA0AD80, (void* _this))

//public Void OnSelectItem(); 
RESOLVE_BY_OFFSET(void, MonoSelectItem_OnSelectItem, 0xCE1C190, (void* _this))

//public MonoReusableListItem get_Item(Int32 
RESOLVE_BY_OFFSET(void*, MonoReusableList_get_Item, 0x81D9080, (void* _this, int32_t index))

//RESOLVE_BY_OFFSET(Unity::Bounds, Action_Invoke, 0x14591C10, (void* _this))

// derives from InLevelManager
//public static Bounds [A-Z]{11}\(GameObject [A-Z]{11}\)
RESOLVE_BY_OFFSET(Unity::Bounds, StageManager_GetBounds, 0x793D6B0, (Unity::GameObject* _this))

//public Transform get_transform(); // FLAGS: 0x886,
RESOLVE_BY_OFFSET(Unity::Transform*, Component_get_transform, 0x14F2F030, (Unity::Component* _this))

//public static Int32 get_height(); // FLAGS: 0x896
RESOLVE_BY_OFFSET(int32_t, Screen_get_height, 0x14F35950, ())
//public static Int32 get_width(); // FLAGS: 0x896,
RESOLVE_BY_OFFSET(int32_t, Screen_get_width, 0x14F35940, ())

//RESOLVE_BY_OFFSET(int32_t, Camera_get_pixelWidth, 0x14C9F370, (Unity::Camera* _this))
//RESOLVE_BY_OFFSET(int32_t, Camera_get_pixelHeight, 0x14C9F380, (Unity::Camera* _this))
//public Vector3 WorldToScreenPoint(Vector3 position); // FLAGS: 0x86
RESOLVE_BY_OFFSET(Unity::Vector3, Camera_WorldToScreenPoint, 0x14F1E420, (Unity::Camera* _this, Unity::Vector3 position))
//public Vector3 WorldToViewportPoint(Vector3 position); // FLAGS: 0x86
RESOLVE_BY_OFFSET(Unity::Vector3, Camera_WorldToViewportPoint, 0x14F1E4D0, (Unity::Camera* _this, Unity::Vector3 position))
//public static Camera get_main(); // FLAGS: 0x896
RESOLVE_BY_OFFSET(Unity::Camera*, Camera_get_main, 0x14F1E800, ())

//RESOLVE_BY_OFFSET(Il2CppClass*, MetadataCache_GetTypeInfoFromTypeDefinitionIndex, 0x446450, (int32_t typeDefinitionIndex))

//RESOLVE_BY_OFFSET(const char*, il2cpp_method_get_name, 0x3DCA60, (MethodInfo* method))
//RESOLVE_BY_OFFSET(Il2CppClass*, il2cpp_class_from_name, 0x43F250, (uintptr_t image, const char* namespaceName, const char* className))
//RESOLVE_BY_OFFSET(uintptr_t, il2cpp_get_corlib, 0x3DBF70, ())

//class: private MonoInLevelMusicPage <
//firstly find EntityManager class name, than search for methods returning that class 
//private static [A-Z]{11} [A-Z]{11}\(\); // FLAGS: 0x891
RESOLVE_BY_OFFSET(MoleMole::EntityManager*, InLevelDrumPageContext_get_ENTITY, 0xE3139B0, ())

//class: private Queue<Dictionary<UInt32,Int32>>
//direct: 48 83 EC ? 48 89 CA 80 3D ? ? ? ? 00 75 ? 48 8B 42 ? 48 83 C4 ? C3 48 8B 05 ? ? ? ? 48 8B 80 ? ? ? ? 48 8B 88 ? ? ? ? 48 85 C9 74 ? 48 83 C4 ? E9 ? ? ? ? E8 ? ? ? ? ? 66 66 66 66 66 2E 0F 1F 84 00 ? ? ? ? 56 57 48 83 EC ? 48 89 D6 48 89 CF 80 3D ? ? ? ? 00 74 ? 80 3D ? ? ? ? 00 75 ? 48 85 F6 74
RESOLVE_BY_OFFSET(Unity::List<MoleMole::BaseEntity*>*, EntityManager_GetEntities, 0x100132F0, (MoleMole::EntityManager* _this))
//public List<[A-Z]{11}> [A-Z]{11}\(\); // FLAGS: 0x86
//xref: E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 89 C7 44 8B 78 ? 48 8D 75
RESOLVE_BY_OFFSET(Unity::List<MoleMole::BaseEntity*>*, EntityManager_GetAllTeams, 0xEAC1C60, (MoleMole::EntityManager* _this))

//public static List<RuntimePlatform> 
//public GameObject [A-Z]{11}\(\); // FLAGS: 0x886
RESOLVE_BY_OFFSET(Unity::GameObject*, BaseEntity_get_gameObject, 0xDA48000, (MoleMole::BaseEntity* _this)) // 0xDA23490
//xref: E8 ? ? ? ? 49 89 C4 8B AE
RESOLVE_BY_OFFSET(Unity::String*, BaseEntity_GetName, 0xDA31E50, (MoleMole::BaseEntity* _this))

//public static IntPtr StringToHGlobalAnsi(String s);
RESOLVE_BY_OFFSET(const char*, Marshal_StringToHGlobalAnsi, 0x14913B40, (Unity::String* string))

//private String CreateString(Char* value);
RESOLVE_BY_OFFSET(Unity::String*, String_CreateString, 0x147A3AC0, (char* value))

//public Transform get_transform(); // FLAGS: 0x886, RVA: 
RESOLVE_BY_OFFSET(Unity::Transform*, GameObject_get_transform, 0x14F0DE40, (Unity::GameObject* _this))

//public Vector3 get_position(); // FLAGS: 0x886, RVA:  
RESOLVE_BY_OFFSET(void, Transform_get_position, 0x14F28630, (Unity::Vector3* _return, Unity::Transform* _this))
