// resolve_funcs.h
#pragma once

#include <cstdint>
#include "../internal_include.h"

extern uintptr_t g_game_base;

#define RESOLVE_BY_OFFSET(return_type, name, offset, params) \
    typedef return_type (*name##_t)params;                                                               \
    extern name##_t name;                                                                                \

#define RESOLVE_BY_SIGNATURE(return_type, name, signature, params) \
    typedef return_type (*name##_t)params;                                                               \
    extern name##_t name;                                                                                \

#define RESOLVE_BY_XREF_SIGNATURE(return_type, name, signature, params) \
    typedef return_type (*name##_t)params;                                                               \
    extern name##_t name;                                                                                \

#include "functions_list.h"

#undef RESOLVE_BY_OFFSET
#undef RESOLVE_BY_SIGNATURE
#undef RESOLVE_BY_XREF_SIGNATURE

void InitSDK();
