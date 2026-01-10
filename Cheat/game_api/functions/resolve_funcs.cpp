// funcs.cpp
#include "resolve_funcs.h"

extern uintptr_t g_game_base;

#define RESOLVE_BY_OFFSET(return_type, name, offset, params) \
    name##_t name = nullptr;

#include "functions_list.h"

#undef RESOLVE_BY_OFFSET

void InitSDK() {
#define RESOLVE_BY_OFFSET(return_type, name, offset, params) \
    name = reinterpret_cast<name##_t>(g_game_base + offset);

#include "functions_list.h"

#undef RESOLVE_BY_OFFSET
}