// common.h
#pragma once

#include <winsock2.h>
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <sstream>
#include <mutex>
#include <algorithm>

// Global Configuration
namespace Config {
    extern uintptr_t GameBase;
    extern bool BlockPackets;
}