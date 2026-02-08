#include "menu.h"
#include "../features/features.h"

#include <imgui/imgui.h>
#include <chrono>

void Menu::Draw() {
	features::DrawAllBackgroundUI();

	// Throttle hotkey updates to once every 500 ms
	static auto lastHotkeyUpdate = std::chrono::steady_clock::now() - std::chrono::milliseconds(500);
	constexpr std::chrono::milliseconds hotkeyInterval(100);
	auto now = std::chrono::steady_clock::now();
	if (now - lastHotkeyUpdate >= hotkeyInterval) {
		features::UpdateAllHotkeys();
		lastHotkeyUpdate = now;
	}

	if (m_IsOpen) {
		ImGui::Begin("SVASTONE: REBIRTH", nullptr);

		features::DrawAllUI();

		ImGui::End();
	}
}
