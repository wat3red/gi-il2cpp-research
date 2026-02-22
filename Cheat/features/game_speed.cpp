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

	// Ќовые переменные дл€ отслеживани€ времени
	static LARGE_INTEGER g_last_real_time = {};
	static LARGE_INTEGER g_accumulated_game_time = {};
	static bool g_time_initialized = false;
	static SRWLOCK g_time_lock = SRWLOCK_INIT;

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
			ImGuiEx::SliderFloat("Speed##game_speed", config.game_speed.speed, 0.1f, 3.f);
			ImGui::Unindent();
		}
	}

	BOOL(*Kernel32_QueryPerformanceCounter)(LARGE_INTEGER* lp);
	BOOL WINAPI hKernel32_QueryPerformanceCounter(LARGE_INTEGER* lp) {
		BOOL result = Kernel32_QueryPerformanceCounter(lp);

		if (!result || IsLocalThread())
			return result;
		
		AcquireSRWLockExclusive(&g_time_lock);

		if (!g_time_initialized) {
			g_last_real_time = *lp;
			g_accumulated_game_time = *lp;
			g_time_initialized = true;
			ReleaseSRWLockExclusive(&g_time_lock);
			return result;
		}

		LONGLONG real_delta = lp->QuadPart - g_last_real_time.QuadPart;

		float speed_multiplier = (config.game_speed.enabled ? config.game_speed.speed : 1.0f);
		LONGLONG game_delta = (LONGLONG)(real_delta * speed_multiplier);

		g_accumulated_game_time.QuadPart += game_delta;
		g_last_real_time = *lp;

		lp->QuadPart = g_accumulated_game_time.QuadPart;

		ReleaseSRWLockExclusive(&g_time_lock);
		return result;
	}

	void(*QualitySettings_set_vSyncCount)(int32_t value);
	void hQualitySettings_set_vSyncCount(int32_t value) {
		if (config.game_speed.enabled)
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