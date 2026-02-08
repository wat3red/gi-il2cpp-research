#include "imgui_config.h"
#include <imgui/imgui_internal.h>

bool ImGuiEx::Combo(const char* label, ConfigVar<int>& currentItem, const char* const items[], int itemsCount) {
	int value = currentItem.GetValue();
	if (ImGui::Combo(label, &value, items, itemsCount)) {
		currentItem = value;
		return true;
	}
	return false;
}

bool ImGuiEx::Checkbox(const char* label, ConfigVar<bool>& var) {
	bool value = var.GetValue();
	if (ImGui::Checkbox(label, &value)) {
		var = value;
		return true;
	}
	return false;
}

bool ImGuiEx::SliderFloat(const char* label, ConfigVar<float>& var, float min, float max) {
	float value = var.GetValue();
	if (ImGui::SliderFloat(label, &value, min, max)) {
		var = value;
		return true;
	}
	return false;
}

bool ImGuiEx::SliderFloat2(const char* label, ConfigVar<ImVec2>& var, float min, float max) {
	ImVec2 value = var.GetValue();
	float vec[3] = { value.x, value.y };

	if (ImGui::SliderFloat2(label, vec, min, max)) {
		var = ImVec2{ vec[0], vec[1] };
		return true;
	}

	return false;
}

bool ImGuiEx::SliderInt(const char* label, ConfigVar<int>& var, int min, int max, const char* format) {
	int value = var.GetValue();
	if (ImGui::SliderInt(label, &value, min, max, format)) {
		var = value;
		return true;
	}
	return false;
}

static const char* ImGuiKeyToString(ImGuiKey key) {
	switch (key) {
	case ImGuiKey_None: return "NONE";
	case ImGuiKey_Tab: return "TAB";
	case ImGuiKey_LeftArrow: return "LEFT ARROW";
	case ImGuiKey_RightArrow: return "RIGHT ARROW";
	case ImGuiKey_UpArrow: return "UP ARROW";
	case ImGuiKey_DownArrow: return "DOWN ARROW";
	case ImGuiKey_Space: return "SPACE";
	case ImGuiKey_Enter: return "ENTER";
	case ImGuiKey_Escape: return "ESCAPE";
	case ImGuiKey_LeftCtrl: return "L CTRL";
	case ImGuiKey_RightCtrl: return "R CTRL";
	case ImGuiKey_LeftShift: return "L SHIFT";
	case ImGuiKey_RightShift: return "R SHIFT";
	case ImGuiKey_LeftAlt: return "L ALT";
	case ImGuiKey_RightAlt: return "R ALT";
	case ImGuiKey_A: return "A";
	case ImGuiKey_B: return "B";
	case ImGuiKey_C: return "C";
	case ImGuiKey_D: return "D";
	case ImGuiKey_E: return "E";
	case ImGuiKey_F: return "F";
	case ImGuiKey_G: return "G";
	case ImGuiKey_H: return "H";
	case ImGuiKey_I: return "I";
	case ImGuiKey_J: return "J";
	case ImGuiKey_K: return "K";
	case ImGuiKey_L: return "L";
	case ImGuiKey_M: return "M";
	case ImGuiKey_N: return "N";
	case ImGuiKey_O: return "O";
	case ImGuiKey_P: return "P";
	case ImGuiKey_Q: return "Q";
	case ImGuiKey_R: return "R";
	case ImGuiKey_S: return "S";
	case ImGuiKey_T: return "T";
	case ImGuiKey_U: return "U";
	case ImGuiKey_V: return "V";
	case ImGuiKey_W: return "W";
	case ImGuiKey_X: return "X";
	case ImGuiKey_Y: return "Y";
	case ImGuiKey_Z: return "Z";

	case ImGuiKey_MouseLeft: return "MOUSE LEFT";
	case ImGuiKey_MouseRight: return "MOUSE RIGHT";
	case ImGuiKey_MouseMiddle: return "MOUSE MIDDLE";
	case ImGuiKey_MouseX1: return "MOUSE X1";
	case ImGuiKey_MouseX2: return "MOUSE X2";

	default: return "UNKNOWN";
	}
}

void ImGuiEx::Hotkey(const char* label, ConfigVar<ImGuiKey>& hotkey) {
	ImGui::Text(label);
	ImGui::SameLine();

	const ImGuiID id = ImGui::GetID(label);
	ImGui::PushID(id);

	// If this widget is active (user clicked it)
	if (ImGui::GetActiveID() == id) {
		ImGui::SetActiveID(id, ImGui::GetCurrentWindow());

		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_ButtonActive));
		ImGui::Button("...");
		ImGui::PopStyleColor();

		// Detect keyboard key press
		for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key++) {
			if (ImGui::IsKeyPressed((ImGuiKey)key)) {
				hotkey = (ImGuiKey)key;
				ImGui::ClearActiveID();
				ImGui::PopID();
				return;
			}
		}

		// Detect mouse clicks
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { hotkey = ImGuiKey_MouseLeft;  ImGui::ClearActiveID(); ImGui::PopID(); return; }
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) { hotkey = ImGuiKey_MouseRight; ImGui::ClearActiveID(); ImGui::PopID(); return; }
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) { hotkey = ImGuiKey_MouseMiddle; ImGui::ClearActiveID(); ImGui::PopID(); return; }

		// Unbind with Escape
		if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
			hotkey = ImGuiKey_None;
			ImGui::ClearActiveID();
			ImGui::PopID();
			return;
		}
	}
	else {
		if (ImGui::Button(ImGuiKeyToString(hotkey))) {
			ImGui::SetActiveID(id, ImGui::GetCurrentWindow());
		}
	}

	ImGui::PopID();
}

bool ImGuiEx::SliderFloat3(const char* label, ConfigVar<ImVec3>& var, float min, float max) {
	ImVec3 value = var.GetValue(); // Fetch current value
	float vec[3] = { value.x, value.y, value.z };

	if (ImGui::SliderFloat3(label, vec, min, max)) {
		var = ImVec3{ vec[0], vec[1], vec[2] }; // Update config variable
		return true;
	}

	return false;
}

bool ImGuiEx::ColorEdit4(const char* label, ConfigVar<ImColor>& var, bool showMainLabel) {
	if (!showMainLabel) {
		ImGui::SameLine();
		ImVec2 min = ImGui::GetCursorPos();
		ImVec2 avail = ImGui::GetContentRegionAvail();
		const ImGuiStyle& style = ImGui::GetStyle();
		ImGui::SetCursorPosX(min.x + avail.x - ImGui::GetFrameHeight() * 2 - style.ItemSpacing.x - style.WindowPadding.x - (ImGui::GetCurrentWindow()->ScrollbarY ? style.ScrollbarSize : 0.f));
	}

	float* value = &var.GetPointer()->Value.x;

	if (ImGui::ColorEdit4(label, value, ImGuiColorEditFlags_NoInputs | (showMainLabel ? ImGuiColorEditFlags_None : ImGuiColorEditFlags_NoLabel))) {
		var = ImColor(value[0], value[1], value[2], value[3]);
		return true;
	}
	return false;
}

bool ImGuiEx::ColorEdit4(const char* label, ConfigVar<ImVec4>& var, bool showMainLabel) {
	if (!showMainLabel) {
		ImGui::SameLine();
		ImVec2 min = ImGui::GetCursorPos();
		ImVec2 avail = ImGui::GetContentRegionAvail();
		const ImGuiStyle& style = ImGui::GetStyle();
		ImGui::SetCursorPosX(min.x + avail.x - ImGui::GetFrameHeight() * 2 - style.ItemSpacing.x - style.WindowPadding.x - (ImGui::GetCurrentWindow()->ScrollbarY ? style.ScrollbarSize : 0.f));
	}

	ImVec4 color = var.GetValue();
	float value[4] = { color.x, color.y, color.z, color.w };
	if (ImGui::ColorEdit4(label, value, ImGuiColorEditFlags_NoInputs | (showMainLabel ? ImGuiColorEditFlags_None : ImGuiColorEditFlags_NoLabel))) {
		var = ImVec4{ value[0], value[1], value[2], value[3] };
		return true;
	}

	return false;
}

