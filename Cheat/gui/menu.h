#pragma once

class Menu {
public:
	void Draw();

	static Menu& GetInstance() {
		static Menu instance;
		return instance;
	}
//private:
	bool m_IsOpen = false;
	bool m_ESP = true;
};
