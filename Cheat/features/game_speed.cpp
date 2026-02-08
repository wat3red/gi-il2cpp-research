#define IMGUI_DEFINE_MATH_OPERATORS

#include "game_speed.h"

#include <game_api/include.h>
#include <logger/logger.h>
#include <config/imgui_config.h>
#include <config/config.h>

#include <imgui/imgui.h>
#include <minhook/include/MinHook.h>
#include <imgui/imgui_internal.h>
#include <unordered_set>

namespace features
{
	static std::unordered_set<DWORD> g_local_threads;
	static SRWLOCK g_thread_lock = SRWLOCK_INIT;

	bool IsLocalThread() {
		DWORD tid = GetCurrentThreadId();
		AcquireSRWLockShared(&g_thread_lock);
		bool ok = g_local_threads.contains(tid);
		ReleaseSRWLockShared(&g_thread_lock);
		return ok;
	}

	void GameSpeed::MarkLocalThread() {
		AcquireSRWLockExclusive(&g_thread_lock);
		g_local_threads.insert(GetCurrentThreadId());
		ReleaseSRWLockExclusive(&g_thread_lock);
	}

	void GameSpeed::DrawUI() {
		ImGuiEx::Checkbox("Enable game speed", config.game_speed.enabled);
		if (config.game_speed.enabled) {
			ImGui::Indent();
			ImGuiEx::SliderFloat("Speed##game_speed", config.game_speed.speed, 1.f, 3.f);
			ImGui::Unindent();
		}
	}

	BOOL(*Kernel32_QueryPerformanceCounter)(LARGE_INTEGER* lp);
	BOOL WINAPI hKernel32_QueryPerformanceCounter(LARGE_INTEGER* lp) {
		BOOL result = Kernel32_QueryPerformanceCounter(lp);
		if (IsLocalThread())
			return result;

		if (result) {
			lp->QuadPart = (LONGLONG)(lp->QuadPart * (config.game_speed.enabled ? config.game_speed.speed : 1.f));
		}

		return result;
	}

	void(*QualitySettings_set_vSyncCount)(int32_t value);
	void hQualitySettings_set_vSyncCount(int32_t value) {
		if (config.game_speed.enabled && value != 0)
			value = 0;

		QualitySettings_set_vSyncCount(value);
	}

	void GameSpeed::OnInit() {
		MH_CreateHookApi(L"Kernel32", "QueryPerformanceCounter",
			&hKernel32_QueryPerformanceCounter, (LPVOID*)(&Kernel32_QueryPerformanceCounter));

		MH_CreateHook(Il2Cpp::Method::GetMethodPointer(Il2Cpp::Method::Find("UnityEngine", "QualitySettings", "set_vSyncCount", 1)),
			hQualitySettings_set_vSyncCount, (LPVOID*)&QualitySettings_set_vSyncCount);
	}
}