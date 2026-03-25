#include "config_manager.h"
#include <logger/logger.h>

std::string ConfigManager::CurrentConfig = ("default.json");

void ConfigManager::Save(const json& config) {
    char* userProfile = nullptr;
    size_t len = 0;
    errno_t err = _dupenv_s(&userProfile, &len, ("USERPROFILE"));

    if (err == 0 && userProfile != nullptr) {
        std::string directory = std::string(userProfile) + ("\\Lucent");
        free(userProfile);  // Clean up allocated memory

        std::filesystem::create_directories(directory);
        std::ofstream file(directory + ("\\") + CurrentConfig);
        if (file.is_open()) {
            file << config.dump(4);
            file.close();
        }
    }
}

json ConfigManager::Load() {
    char* userProfile = nullptr;
    size_t len = 0;
    errno_t err = _dupenv_s(&userProfile, &len, "USERPROFILE");

    if (err != 0 || userProfile == nullptr) {
        return json{};
    }

    std::string directory = std::string(userProfile) + "\\Lucent";
    free(userProfile);

    std::ifstream file(directory + "\\" + CurrentConfig);
    if (!file.is_open()) return json{};

    try {
        json config;
        file >> config;
        return config;
    }
    catch (const json::parse_error& e) {
        Log("JSON parse error: %s\n", e.what());
        return json{};
    }
    catch (...) {
        Log("Unknown exception in ConfigManager::Load\n");
        return json{};
    }
}

std::vector<std::string> ConfigManager::SplitPath(const std::string& path) {
    std::vector<std::string> result;
    size_t start = 0, end;
    while ((end = path.find(("."), start)) != std::string::npos) {
        result.push_back(path.substr(start, end - start));
        start = end + 1;
    }
    result.push_back(path.substr(start));
    return result;
}

void ConfigManager::RemoveKey(const std::string& path, const std::string& key) {
    json config = Load();

    std::vector<std::string> parts = SplitPath(path);
    std::vector<json*> stack;

    json* section = &config;
    stack.push_back(section);

    for (const auto& part : parts) {
        if (!section->contains(part))
            return;

        section = &((*section)[part]);
        stack.push_back(section);
    }

    section->erase(key);

    // Clean up empty parents (bottom-up)
    for (int i = (int)stack.size() - 1; i > 0; --i) {
        json* current = stack[i];
        json* parent = stack[i - 1];

        if (current->empty()) {
            parent->erase(parts[i - 1]);
        }
        else {
            break;
        }
    }

    Save(config);
}

void ConfigManager::RemoveSection(const std::string& path) {
    json config = Load();
    json* section = &config;
    std::vector<std::string> parts = SplitPath(path);
    if (parts.empty())
        return;

    for (size_t i = 0; i < parts.size() - 1; ++i) {
        if (!section->contains(parts[i]))
            return;
        section = &((*section)[parts[i]]);
    }

    section->erase(parts.back());
    Save(config);
}