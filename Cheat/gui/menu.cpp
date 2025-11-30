#include "menu.h"
#include "../features/features.h"

#include <imgui/imgui.h>

void Menu::Draw() {
	features::DrawAllBackgroundUI();

	if (m_IsOpen) {
		ImGui::Begin("SVASTONE: REBIRTH", nullptr);

		features::DrawAllUI();

		ImGui::End();
	}
}
