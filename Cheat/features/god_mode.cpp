#include "god_mode.h"

namespace features
{
	void GodMode::DrawUI() {
		ImGuiEx::Checkbox("Enable godmode", config.god_mode.enabled);
	}

	void (*LCBaseCombat_FireBeingHitEvent)(void* _this, uint32_t attackeeId, void* attackResult);
	void hLCBaseCombat_FireBeingHitEvent(void* _this, uint32_t attackeeId, void* attackResult) {
		MoleMole::AvatarEntity* avatar = MoleMole::EntityManager::Instance()->GetAvatar();

		Log("0x10:%s\n", (*(Il2CppString**)((uintptr_t)attackResult + 0x10))->ToCStr());
		Log("0x18:%s\n", (*(Il2CppString**)((uintptr_t)attackResult + 0x18))->ToCStr());
		Log("0x20:%s\n", (*(Il2CppString**)((uintptr_t)attackResult + 0x20))->ToCStr());
		Log("0x48:%s\n", (*(Il2CppString**)((uintptr_t)attackResult + 0x48))->ToCStr());
		Log("0x60:%s\n", (*(Il2CppString**)((uintptr_t)attackResult + 0x60))->ToCStr());

		Log("0x120:%f\n", *(float*)((uintptr_t)attackResult + 0x120));
		Log("0x13C:%f\n", *(float*)((uintptr_t)attackResult + 0x13C));
		Log("0x140:%f\n", *(float*)((uintptr_t)attackResult + 0x140)); //
		Log("0x158:%f\n", *(float*)((uintptr_t)attackResult + 0x158));
		Log("0x1A8:%f\n", *(float*)((uintptr_t)attackResult + 0x1A8)); // bulletFlyTime

		Log("0x244:%f\n", *(float*)((uintptr_t)attackResult + 0x244));
		Log("0x248:%f\n", *(float*)((uintptr_t)attackResult + 0x248));
		Log("0x250:%f\n", *(float*)((uintptr_t)attackResult + 0x250));
		Log("0x258:%f\n", *(float*)((uintptr_t)attackResult + 0x258));
		Log("0x280:%f\n", *(float*)((uintptr_t)attackResult + 0x280)); // 

		//*(float*)((uintptr_t)attackResult + 0x140) = 100.f;
		*(float*)((uintptr_t)attackResult + 0x280) = 100.f;

		if (avatar &&
			config.god_mode.enabled &&
			attackeeId == avatar->GetRuntimeID())
			return;

		LCBaseCombat_FireBeingHitEvent(_this, attackeeId, attackResult);
	}

	void (*VCHumanoidMove_NotifyLandVelocity)(void* _this, Unity::Vector3 velocity, float reachMaxDownVelocityTime);
	void hVCHumanoidMove_NotifyLandVelocity(void* _this, Unity::Vector3 velocity, float reachMaxDownVelocityTime) {
		if (config.god_mode.enabled)
			return;

		VCHumanoidMove_NotifyLandVelocity(_this, velocity, reachMaxDownVelocityTime);
	}


	void GodMode::OnInit() {
		// MoleMole.LCBaseCombat : "public Action<Single,Single,Boolean"

		// "41 56 56 57 55 53 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 4C 89 C3 41 89 D6"
		MH_CreateHook((LPVOID)(Mem::Signature("41 56 56 57 53 48 83 EC ? 4D 89 C6 89 D3 48 89 CF 80 3D ? ? ? ? 00 75 ? 48 8B 0D").Scan()),
			hLCBaseCombat_FireBeingHitEvent, (LPVOID*)&LCBaseCombat_FireBeingHitEvent);

		// public void [A-Z]{11}\(Vector3 [A-Z]{11}, single [A-Z]{11}\)
		//"48 83 EC ? 49 89 C9 80 3D ? ? ? ? 00 75 ? 49 8B 89 ? ? ? ? 48 85 C9 74 ? 8B 42 ? 89 44 24 ? ? ? ? 48 89 44 24 ? 48 8D 54 24 ? E8 ? ? ? ? 90 48 83 C4 ? C3 48 8B 0D ? ? ? ? 48 8B 89 ? ? ? ? 48 8B 89 ? ? ? ? 48 85 C9 74 ? 8B 42 ? 89 44 24 ? ? ? ? 48 89 44 24 ? 4C 8D 44 24 ? 4C 89 CA 0F 28 DA"
		MH_CreateHook((LPVOID)(Mem::Signature("41 56 56 57 53 48 81 EC ? ? ? ? 0F 29 BC 24 ? ? ? ? 0F 29 B4 24 ? ? ? ? 0F 28 F2 49 89 D6").Scan()),
			hVCHumanoidMove_NotifyLandVelocity, (LPVOID*)&VCHumanoidMove_NotifyLandVelocity);
	}
}