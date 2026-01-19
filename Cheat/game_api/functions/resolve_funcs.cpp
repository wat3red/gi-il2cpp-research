// funcs.cpp
#include "resolve_funcs.h"
#include <game_api/memory/signature.h>

extern uintptr_t g_game_base;

#define RESOLVE_BY_OFFSET(return_type, name, offset, params) \
    name##_t name = nullptr;

#define RESOLVE_BY_SIGNATURE(return_type, name, signature, params) \
    name##_t name = nullptr;

#define RESOLVE_BY_XREF_SIGNATURE(return_type, name, signature, params) \
    name##_t name = nullptr;

#include "functions_list.h"

#undef RESOLVE_BY_OFFSET
#undef RESOLVE_BY_SIGNATURE
#undef RESOLVE_BY_XREF_SIGNATURE

void InitSDK() {
#define RESOLVE_BY_OFFSET(return_type, name, offset, params) \
    name = reinterpret_cast<name##_t>(g_game_base + offset);

#define RESOLVE_BY_SIGNATURE(return_type, name, signature, params) \
    name = reinterpret_cast<name##_t>(Mem::Signature(signature).Scan());

#define RESOLVE_BY_XREF_SIGNATURE(return_type, name, signature, params) \
    name = reinterpret_cast<name##_t>(Mem::Signature(signature).ScanXref());

#include "functions_list.h"

#undef RESOLVE_BY_OFFSET
#undef RESOLVE_BY_SIGNATURE
#undef RESOLVE_BY_XREF_SIGNATURE
}