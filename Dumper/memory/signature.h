#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Mem {

    class Signature {
    public:
        Signature() = default;
        explicit Signature(const std::string& pattern);

        bool Valid() const;

        void* Scan(const uint8_t* base = 0, size_t size = 0) const;
        void* ScanXref(const uint8_t* base = 0, size_t size = 0) const;

    private:
        void Parse(const std::string& pattern);

    private:
        const std::string& m_pattern;
        std::vector<uint8_t> m_bytes;
        std::vector<uint8_t> m_mask; // 0xFF = match, 0x00 = wildcard
    };

}
