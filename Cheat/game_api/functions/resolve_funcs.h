// resolve_funcs.h
#pragma once

#include <cstdint>
#include "../internal_include.h"

extern uintptr_t g_game_base;

#define RESOLVE_BY_OFFSET(return_type, name, offset, params) \
    typedef return_type (*name##_t)params;                                                               \
    extern name##_t name;                                                                                \

#include "functions_list.h"

#undef RESOLVE_BY_OFFSET

void InitSDK();
