#include "offset_db.h"

int32_t Mem::OffsetDB::Resolve(std::string name, const char* sig) {
    static std::unordered_map<std::string, int32_t> cache;
    static std::mutex lock;

    std::lock_guard<std::mutex> _{ lock };

    auto it = cache.find(name);
    if (it != cache.end())
        return it->second;

    Signature s(sig);
    int32_t val = s.FindDisp();

    cache[name] = val;

    if (!val)
        Log("[OffsetDB] %s NOT FOUND\n", name.c_str());
    else
        Log("[OffsetDB] %s = 0x%X\n", name.c_str(), val);

    return val;
}
