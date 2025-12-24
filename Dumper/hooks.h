// hooks.h
#pragma once
#include "common.h"
#include "lib/minhook/include/MinHook.h" // Ensure path is correct

namespace Hooks {
    bool Init();
    void Uninit();
}