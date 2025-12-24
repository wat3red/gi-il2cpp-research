// utils.cpp
#include "utils.h"

namespace Utils {

    void CreateConsole() {
        AllocConsole();
        FILE* fDummy;
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONIN$", "r", stdin);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        std::ios::sync_with_stdio(true);
    }

    void DisableLogReport() {
        wchar_t filename[MAX_PATH] = {};
        GetModuleFileNameW(NULL, filename, MAX_PATH);

        auto path = std::filesystem::path(filename);
        path = path.parent_path() / (path.stem().string() + "_Data") / "Plugins";

        // Lock report DLLs to prevent loading
        CreateFileW((path / "Astrolabe.dll").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        CreateFileW((path / "MiHoYoMTRSDK.dll").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    std::string StripNamespaces(const std::string& full) {
        std::string out;
        out.reserve(full.size());

        for (size_t i = 0; i < full.size(); ++i) {
            char c = full[i];

            // If we encounter a name inside generics (< ... >)
            if (std::isalnum((unsigned char)c) || c == '_') {
                size_t start = i;

                // Read token (until < > , . whitespace)
                while (i < full.size() &&
                    (std::isalnum((unsigned char)full[i]) || full[i] == '_' || full[i] == '.')) {
                    i++;
                }

                std::string token = full.substr(start, i - start);

                // If namespace exists -> cut everything before the last dot
                size_t dot = token.rfind('.');
                if (dot != std::string::npos)
                    token = token.substr(dot + 1);

                out += token;
                i--; // Compensate loop increment
                continue;
            }
            out.push_back(c);
        }
        return out;
    }

    std::string SanitizeName(std::string name) {
        std::string out;
        for (char c : name) {
            if (isalnum(c) || c == '_') {
                out += c;
            }
            else {
                out += "_";
            }
        }

        // Remove consecutive underscores
        std::string clean;
        bool lastUnder = false;
        for (char c : out) {
            if (c == '_') {
                if (!lastUnder) clean += c;
                lastUnder = true;
            }
            else {
                clean += c;
                lastUnder = false;
            }
        }
        return clean;
    }
}