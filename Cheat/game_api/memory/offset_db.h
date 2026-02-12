#pragma once
#include <unordered_map>
#include <string>
#include <mutex>

#include "signature.h"
#include <logger/logger.h>

namespace Mem
{
    class OffsetDB {
    public:
        static int32_t Resolve(std::string name, const char* sig);
    };
}

#define DEFINE_OFFSET(name, sig) \
    inline int32_t name() { \
        static int32_t v = Mem::OffsetDB::Resolve(#name, sig); \
        return v; \
    }
