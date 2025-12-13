//public virtual Void ClosePage();
RESOLVE_BY_OFFSET(void, BasePageContext_ClosePage, 0x736BAC0, (void* _this))

//public static Single CalcCurrentGroundHeight(Single x, Single z); // Slot: -1, RVA: 0xF239F40, FLAGS: 0x96
RESOLVE_BY_OFFSET(float, Miscs_CalcCurrentGroundHeight, 0xF239F40, (float x, float z))

//public static Vector3d GetRelativePosition(Vector3d pos); // Slot: -1, RVA: 0x5D906E0, FLAGS: 0x96
RESOLVE_BY_OFFSET(Unity::Vector3, WorldShiftManager_GetRelativePosition, 0x5D906E0, (Unity::Vector3 pos))

//public static Vector3 GenWorldPos(Vector2 levelMapPos); // Slot: -1, RVA: 0xF26AAF0, FLAGS: 0x96
RESOLVE_BY_OFFSET(Unity::Vector3, Miscs_GenWorldPos, 0xF26AAF0, (Unity::Vector2 levelMapPos))

//public static Boolean ScreenPointToLocalPointInRectangle(RectTransform rect, Vector2 screenPoint, Camera cam, Vector2& localPoint); // Slot: -1, RVA: 0x14D55D90, FLAGS: 0x96
RESOLVE_BY_OFFSET(bool, RectTransformUtility_ScreenPointToLocalPointInRectangle, 0x14D55D90, (void* rect, Unity::Vector2 screenPoint, Unity::Camera* cam, Unity::Vector2& localPoint))

//public Transform get_mapBackground(); // Slot: -1, RVA: 0xC317C20, FLAGS: 0x886
RESOLVE_BY_OFFSET(Unity::Transform*, MonoInLevelMapPage_get_mapBackground, 0xC317C20, (void* _this))
//public Rect get_mapRect(); // Slot: -1, RVA: 0xC317BE0, FLAGS: 0x886
RESOLVE_BY_OFFSET(MoleMole::Rect, MonoInLevelMapPage_get_mapRect, 0xC317BE0, (void* _this))

RESOLVE_BY_OFFSET(Il2CppObject*, SingletonManager_GetSingletonInstance, 0x5BBAF50, (MoleMole::SingletonManager* _this, Unity::String* typeName))
RESOLVE_BY_OFFSET(MoleMole::SingletonManager*, SingletonManager_get_Instance, 0x5BBB080, ())

RESOLVE_BY_OFFSET(void, HDIKLPILBAC_CFDHCLDPCHI, 0xE6758C0, (void* _this))

//public Void OnSelectItem(); // Slot: -1, RVA: 0xCCC2E30, FLAGS: 0x86
RESOLVE_BY_OFFSET(void, MonoSelectItem_OnSelectItem, 0xCCC2E30, (void* _this))

RESOLVE_BY_OFFSET(void*, MonoReusableList_get_Item, 0x8040AE0, (void* _this, int32_t index))

RESOLVE_BY_OFFSET(Unity::Bounds, Action_Invoke, 0x14591C10, (void* _this))

//public static Bounds [A-Z]{11}\(GameObject [A-Z]{11}\)
RESOLVE_BY_OFFSET(Unity::Bounds, StageManager_GetBounds, 0x61A6140, (Unity::GameObject* _this))

RESOLVE_BY_OFFSET(Unity::Transform*, Component_get_transform, 0x14CB0640, (Unity::Component* _this))

RESOLVE_BY_OFFSET(int32_t, Screen_get_height, 0x14CB6F90, ())
RESOLVE_BY_OFFSET(int32_t, Screen_get_width, 0x14CB6F80, ())

//RESOLVE_BY_OFFSET(int32_t, Camera_get_pixelWidth, 0x14C9F370, (Unity::Camera* _this))
//RESOLVE_BY_OFFSET(int32_t, Camera_get_pixelHeight, 0x14C9F380, (Unity::Camera* _this))
RESOLVE_BY_OFFSET(Unity::Vector3, Camera_WorldToScreenPoint, 0x14C9F9F0, (Unity::Camera* _this, Unity::Vector3 position))
RESOLVE_BY_OFFSET(Unity::Vector3, Camera_WorldToViewportPoint, 0x14C9FB00, (Unity::Camera* _this, Unity::Vector3 position))
//RESOLVE_BY_OFFSET(Unity::Camera*, Camera_get_current, 0x14C9FE40, ())
RESOLVE_BY_OFFSET(Unity::Camera*, Camera_get_main, 0x14C9FE30, ())

RESOLVE_BY_OFFSET(Il2CppClass*, MetadataCache_GetTypeInfoFromTypeDefinitionIndex, 0x446450, (int32_t typeDefinitionIndex))

RESOLVE_BY_OFFSET(const char*, il2cpp_method_get_name, 0x3DCA60, (MethodInfo* method))
RESOLVE_BY_OFFSET(Il2CppClass*, il2cpp_class_from_name, 0x43F250, (uintptr_t image, const char* namespaceName, const char* className))
RESOLVE_BY_OFFSET(uintptr_t, il2cpp_get_corlib, 0x3DBF70, ())

RESOLVE_BY_OFFSET(MoleMole::EntityManager*, InLevelDrumPageContext_get_ENTITY, 0xC427720, ())

RESOLVE_BY_OFFSET(Unity::List<MoleMole::BaseEntity*>*, EntityManager_GetEntities, 0x100132F0, (MoleMole::EntityManager* _this))
RESOLVE_BY_OFFSET(Unity::List<MoleMole::BaseEntity*>*, EntityManager_GetAllTeams, 0x100258B0, (MoleMole::EntityManager* _this))

RESOLVE_BY_OFFSET(Unity::GameObject*, BaseEntity_get_gameObject, 0xC7F3490, (MoleMole::BaseEntity* _this))
RESOLVE_BY_OFFSET(Unity::String*, BaseEntity_GetName, 0xC807F00, (MoleMole::BaseEntity* _this))

RESOLVE_BY_OFFSET(const char*, Marshal_StringToHGlobalAnsi, 0x3F35E0, (Unity::String* string))

// private String CreateString(Char* value); // Slot: -1, RVA: 0x14523AF0, FLAGS: 0x81
RESOLVE_BY_OFFSET(Unity::String*, String_CreateString, 0x14523AF0, (char* value))

RESOLVE_BY_OFFSET(Unity::Transform*, GameObject_GetTransform, 0x1025420, (Unity::GameObject* _this))

RESOLVE_BY_OFFSET(void, Transform_GetPosition, 0x14CA9CB0, (Unity::Vector3* _return, Unity::Transform* _this))
