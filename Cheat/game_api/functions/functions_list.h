//public virtual Void ClosePage();
RESOLVE_BY_XREF_SIGNATURE(void, BasePageContext_ClosePage, "E8 ? ? ? ? E9 ? ? ? ? 66 0F EF F6", (void* _this))

//public static Single CalcCurrentGroundHeight(Single x, Single z);
//RESOLVE_BY_OFFSET(float, Miscs_CalcCurrentGroundHeight, 0xF4A9EB0, (float x, float z))

//public static Vector3d GetRelativePosition(Vector3d pos);
//RESOLVE_BY_OFFSET(Unity::Vector3, WorldShiftManager_GetRelativePosition, 0x5E74CD0, (Unity::Vector3 pos))

//public static Vector3 GenWorldPos(Vector2 levelMapPos);
//RESOLVE_BY_OFFSET(Unity::Vector3, Miscs_GenWorldPos, 0xF49E820, (Unity::Vector2 levelMapPos))

//public static Boolean ScreenPointToLocalPointInRectangle(RectTransform rect, Vector2 screenPoint, Camera cam, Vector2& localPoint);
//RESOLVE_BY_OFFSET(bool, RectTransformUtility_ScreenPointToLocalPointInRectangle, 0x14FD4890, (void* rect, Unity::Vector2 screenPoint, Unity::Camera* cam, Unity::Vector2& localPoint))

//public Transform get_mapBackground();
//RESOLVE_BY_OFFSET(Unity::Transform*, MonoInLevelMapPage_get_mapBackground, 0xC458930, (void* _this))
//public Rect get_mapRect(); // FLAGS: 0x886
//RESOLVE_BY_OFFSET(Unity::Rect, MonoInLevelMapPage_get_mapRect, 0xC459090, (void* _this))

//public Object GetSingletonInstance(String typeName); 
//RESOLVE_BY_OFFSET(Il2CppObject*, SingletonManager_GetSingletonInstance, 0x5CA0A30, (MoleMole::SingletonManager* _this, Il2CppString* typeName))
// public static SingletonManager get_Instance(); 
//RESOLVE_BY_OFFSET(MoleMole::SingletonManager*, SingletonManager_get_Instance, 0x5CA0B60, ())

RESOLVE_BY_XREF_SIGNATURE(void, HDIKLPILBAC_CFDHCLDPCHI, "E8 ? ? ? ? 48 89 F1 E8 ? ? ? ? 80 BE ? ? ? ? 00 0F 84 ? ? ? ? 80 BE ? ? ? ? 00 74", (void* _this))

//public Void OnSelectItem(); 
//RESOLVE_BY_OFFSET(void, MonoSelectItem_OnSelectItem, 0xCE1C190, (void* _this))

//public MonoReusableListItem get_Item(Int32 
//RESOLVE_BY_OFFSET(void*, MonoReusableList_get_Item, 0x81D9080, (void* _this, int32_t index))

//RESOLVE_BY_OFFSET(Unity::Bounds, Action_Invoke, 0x14591C10, (void* _this))

// derives from InLevelManager
//public static Bounds [A-Z]{11}\(GameObject [A-Z]{11}\)
RESOLVE_BY_XREF_SIGNATURE(Unity::Bounds, StageManager_GetBounds, "E8 ? ? ? ? 48 8B 44 24 ? 48 89 45 ? 0F 10 44 24 ? 0F 11 45 ? 48 8B 4D", (Unity::GameObject* _this))

//public Transform get_transform(); // FLAGS: 0x886,
//RESOLVE_BY_OFFSET(Unity::Transform*, Component_get_transform, 0x14F2F030, (Unity::Component* _this))

//public static Int32 get_height(); // FLAGS: 0x896
//RESOLVE_BY_OFFSET(int32_t, Screen_get_height, 0x14F35950, ())
//public static Int32 get_width(); // FLAGS: 0x896,
//RESOLVE_BY_OFFSET(int32_t, Screen_get_width, 0x14F35940, ())

//RESOLVE_BY_OFFSET(int32_t, Camera_get_pixelWidth, 0x14C9F370, (Unity::Camera* _this))
//RESOLVE_BY_OFFSET(int32_t, Camera_get_pixelHeight, 0x14C9F380, (Unity::Camera* _this))
//public Vector3 WorldToScreenPoint(Vector3 position); // FLAGS: 0x86
//RESOLVE_BY_OFFSET(Unity::Vector3, Camera_WorldToScreenPoint, 0x14F1E420, (Unity::Camera* _this, Unity::Vector3 position))
//public Vector3 WorldToViewportPoint(Vector3 position); // FLAGS: 0x86
//RESOLVE_BY_OFFSET(Unity::Vector3, Camera_WorldToViewportPoint, 0x14F1E4D0, (Unity::Camera* _this, Unity::Vector3 position))
//public static Camera get_main(); // FLAGS: 0x896
//RESOLVE_BY_OFFSET(Unity::Camera*, Camera_get_main, 0x14F1E800, ())

//class: private MonoInLevelMusicPage <
//firstly find EntityManager class name, than search for methods returning that class 
//private static [A-Z]{11} [A-Z]{11}\(\);
//RESOLVE_BY_OFFSET(MoleMole::PlayerModule*, InLevelDrumPageContext_get_PLAYER, 0x92B01B0, ()) // "private static MonoUIWaterMask"
//RESOLVE_BY_OFFSET(MoleMole::EntityManager*, InLevelDrumPageContext_get_ENTITY, 0x92B1B00, ()) // "private Queue<Dictionary<UInt32,Int32>>"

//RESOLVE_BY_SIGNATURE(void, BaseEntity_SetAbsolutePosition, "56 57 55 53 48 83 EC ? 44 89 C5 48 89 D7 48 89 CE 80 3D ? ? ? ? 00 75 ? 8B 47", (MoleMole::BaseEntity* _this, Unity::Vector3 position, bool forceSyncToRigidbody))

RESOLVE_BY_SIGNATURE(void, LoadingManager_RequestSceneTransToPoint, "41 56 56 57 55 53 48 83 EC ? 4C 89 CF 45 89 C6 89 D3 48 89 CE 80 3D ? ? ? ? 00 0F 84 ? ? ? ? 80 3D ? ? ? ? 00 0F 85 ? ? ? ? 80 3D ? ? ? ? 00", (MoleMole::LoadingManager* _this, uint32_t sceneId, uint32_t pointId, void* finishCallBackInForceDrag))

//class: private Queue<Dictionary<UInt32,Int32>>
// 0xE42C660, 0xE42FD90, 0xE437590
//public List<[A-Z]{11}> [A-Z]{11}\(\);
RESOLVE_BY_OFFSET(Unity::List<MoleMole::BaseEntity*>*, EntityManager_GetEntities, 0xE42FD90, (MoleMole::EntityManager* _this))
//public List<[A-Z]{11}> [A-Z]{11}\(\);
RESOLVE_BY_XREF_SIGNATURE(Unity::List<MoleMole::BaseEntity*>*, EntityManager_GetAllTeams, "E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 89 C7 44 8B 78 ? 48 8D 75", (MoleMole::EntityManager* _this))

// should be moved to signature
RESOLVE_BY_OFFSET(MoleMole::AvatarEntity*, EntityManager_GetLocalAvatarEntity, 0xE425B00, (MoleMole::EntityManager* _this)) // 0xE423A90

RESOLVE_BY_SIGNATURE(MoleMole::BaseActor*, ActorManager_GetActor, "41 57 41 56 41 54 56 57 53 48 81 EC ? ? ? ? 48 89 D6 48 89 C8", (MoleMole::ActorManager* _this, MoleMole::BaseEntity* tarEntity))

//public static List<RuntimePlatform> 
//public GameObject [A-Z]{11}\(\);
RESOLVE_BY_OFFSET(Unity::GameObject*, BaseEntity_get_gameObject, 0x87D5680, (MoleMole::BaseEntity* _this)) // 0x87D61E0
RESOLVE_BY_XREF_SIGNATURE(Il2CppString*, BaseEntity_GetName, "E8 ? ? ? ? 49 89 C4 8B AE ? ? ? ? 48 8B 8F", (MoleMole::BaseEntity* _this))





//public static IntPtr StringToHGlobalAnsi(String s);
//RESOLVE_BY_OFFSET(const char*, Marshal_StringToHGlobalAnsi, 0x14913B40, (Il2CppString* string))

//public Transform get_transform(); // FLAGS: 0x886, RVA: 
//RESOLVE_BY_OFFSET(Unity::Transform*, GameObject_get_transform, 0x14F0DE40, (Unity::GameObject* _this))

//public Vector3 get_position(); // FLAGS: 0x886, RVA:  
//RESOLVE_BY_OFFSET(void, Transform_get_position, 0x14F28630, (Unity::Vector3* _return, Unity::Transform* _this))


// ------------------ Il2Cpp API ------------------

RESOLVE_BY_XREF_SIGNATURE(Il2CppClass*, MetadataCache_GetTypeInfoFromTypeDefinitionIndex, "E8 ? ? ? ? 0F B7 A8", (int32_t typeDefinitionIndex))
RESOLVE_BY_SIGNATURE(Il2CppString*, il2cpp_string_new_len, "55 41 57 41 56 56 57 53 48 83 EC ? 48 8D 6C 24 ? 48 C7 45 ? ? ? ? ? 49 89 CF 41 89 D6", (const char* str, uint32_t length))
RESOLVE_BY_XREF_SIGNATURE(const char*, il2cpp_class_get_name, "E8 ? ? ? ? 45 33 F6 C7 85", (Il2CppClass* klass))
RESOLVE_BY_XREF_SIGNATURE(const char*, il2cpp_class_get_namespace, "E8 ? ? ? ? 49 C7 C7 ? ? ? ? 4D 8B C7", (Il2CppClass* klass))
RESOLVE_BY_SIGNATURE(FieldInfo*, il2cpp_class_get_field_from_name,
	"55 41 57 41 56 41 55 41 54 56 57 53 48 83 EC ? 48 8D 6C 24 ? 48 C7 45 ? ? ? ? ? 48 85 C9 0F 84 ? ? ? ? 48 89 D6", (Il2CppClass* klass, const char* name));
RESOLVE_BY_XREF_SIGNATURE(MethodInfo*, il2cpp_class_get_methods, "E8 ? ? ? ? 48 85 C0 74 ? 48 8D 5D", (Il2CppClass* klass, void** iter));
RESOLVE_BY_XREF_SIGNATURE(const char*, il2cpp_method_get_name, "E8 ? ? ? ? 48 8B CE 48 2B C6", (MethodInfo* method));
RESOLVE_BY_XREF_SIGNATURE(uint8_t, il2cpp_method_get_param_count, "E8 ? ? ? ? 3B C5 75", (MethodInfo* method));
RESOLVE_BY_XREF_SIGNATURE(Il2CppImage*, il2cpp_assembly_get_image, "E8 ? ? ? ? 48 8B C8 EB ? 33 C9 ? ? ? ? ? ? ? 48 85 C9", (Il2CppAssembly* assembly));
RESOLVE_BY_XREF_SIGNATURE(Il2CppClass*, il2cpp_class_from_name, "E8 ? ? ? ? 48 83 C4 ? 48 89 05", (Il2CppImage* image, const char* namespaze, const char* name))
RESOLVE_BY_XREF_SIGNATURE(int32_t, il2cpp_field_get_offset, "E8 ? ? ? ? 49 03 45", (FieldInfo* field))

