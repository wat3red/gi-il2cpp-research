#include "signature.h"
#include <logger/logger.h>

#include <Windows.h>
#include <emmintrin.h>
#include <winnt.h>
#include <cstring>
#include <cctype>
#include <thread>
#include <vector>
#include <atomic>

namespace Mem {

	struct Section {
		uint8_t* base;
		size_t   size;

		bool Valid() const { return base && size; }
	};

	static inline bool IsCodeSection(const IMAGE_SECTION_HEADER& s) {
		// executable code only
		if (!(s.Characteristics & IMAGE_SCN_MEM_EXECUTE))
			return false;

		// optional name filtering
		if (strcmp((char*)s.Name, ".text") == 0)
			return true;

		if (strcmp((char*)s.Name, "il2cpp") == 0)
			return true;

		return false;
	}

	static std::vector<Section> GetExecutableSections(HMODULE module)
	{
		std::vector<Section> out;

		if (!module)
			return out;

		auto dos = (PIMAGE_DOS_HEADER)module;
		if (dos->e_magic != IMAGE_DOS_SIGNATURE)
			return out;

		auto nt = (PIMAGE_NT_HEADERS)(
			(uint8_t*)module + dos->e_lfanew);

		if (nt->Signature != IMAGE_NT_SIGNATURE)
			return out;

		auto section = IMAGE_FIRST_SECTION(nt);

		for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section) {
			if (!IsCodeSection(*section))
				continue;

			size_t size =
				section->Misc.VirtualSize
				? section->Misc.VirtualSize
				: section->SizeOfRawData;

			if (!size)
				continue;

			out.push_back({
				(uint8_t*)module + section->VirtualAddress,
				size
				});
		}

		return out;
	}

	// Fast hex digit to value conversion
	static constexpr uint8_t HexTable[256] = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};

	static inline uint8_t HexToUint8(char hi, char lo) {
		return (HexTable[(unsigned char)hi] << 4) | HexTable[(unsigned char)lo];
	}

	Signature::Signature(const std::string& pattern) : m_pattern(pattern) {
		Parse(pattern);
	}

	bool Signature::Valid() const {
		return !m_bytes.empty() && m_bytes.size() == m_mask.size();
	}

	void Signature::Parse(const std::string& pattern) {
		m_bytes.clear();
		m_mask.clear();

		for (size_t i = 0; i < pattern.size(); ++i) {
			if (pattern[i] == '?') {
				m_bytes.push_back(0);
				m_mask.push_back(0x00);
			}
			else if (isspace((unsigned char)pattern[i])) {
				continue;
			}
			else if (i + 1 < pattern.size() &&
				isxdigit(pattern[i]) &&
				isxdigit(pattern[i + 1])) {
				m_bytes.push_back(HexToUint8(pattern[i], pattern[i + 1]));
				m_mask.push_back(0xFF);
				++i;
			}
		}
	}

	static inline bool match16(__m128i cmp, __m128i mask) {
		__m128i r = _mm_and_si128(cmp, mask);
		return _mm_movemask_epi8(r) == _mm_movemask_epi8(mask);
	}

	void* Signature::ScanRange(const uint8_t* base, size_t size) const {
		if (!Valid() || !base || size < m_bytes.size())
			return nullptr;

		const size_t len = m_bytes.size();
		const size_t simd = std::min<size_t>(16, len);
		const size_t end = size - len;

		alignas(16) uint8_t p[16]{};
		alignas(16) uint8_t m[16]{};

		for (size_t i = 0; i < simd; ++i) {
			p[i] = m_bytes[i];
			m[i] = m_mask[i];
		}

		__m128i vpat = _mm_load_si128((__m128i*)p);
		__m128i vmsk = _mm_load_si128((__m128i*)m);

		const uint8_t first_byte = m_bytes[0];
		const bool first_masked = (m_mask[0] == 0x00);

		const size_t maxSimd = (size >= 16) ? size - 16 : 0;
		const size_t scanEnd = std::min<size_t>(end, maxSimd);

		for (size_t i = 0; i <= scanEnd; ++i) {
			// Quick rejection: skip if first byte doesn't match (unless masked)
			if (!first_masked && base[i] != first_byte)
				continue;

			_mm_prefetch((const char*)(base + i + 64), _MM_HINT_T0);

			__m128i mem = _mm_loadu_si128((__m128i*)(base + i));
			__m128i cmp = _mm_cmpeq_epi8(mem, vpat);

			if (!match16(cmp, vmsk))
				continue;

			// Full pattern match
			bool ok = true;
			for (size_t j = simd; j < len; ++j) {
				if (m_mask[j] && base[i + j] != m_bytes[j]) {
					ok = false;
					break;
				}
			}

			if (ok)
				return (void*)(base + i);
		}

		return nullptr;
	}

	void* Signature::Scan(const uint8_t* base, size_t size) const {
		// explicit range: scan exactly what caller asked
		if (base && size)
			return ScanRange(base, size);

		// otherwise scan all executable sections in parallel
		HMODULE game = GetModuleHandleW(nullptr);
		auto sections = GetExecutableSections(game);

		if (sections.empty()) {
			Log("Didn't find signature '%s'\n", m_pattern.c_str());
			return nullptr;
		}

		std::atomic<void*> result(nullptr);
		std::vector<std::thread> threads;
		
		const size_t numThreads = std::min<size_t>(
			std::thread::hardware_concurrency(),
			sections.size()
		);

		// Launch worker threads
		for (size_t t = 0; t < numThreads && !result; ++t) {
			threads.emplace_back([this, &sections, &result, t, numThreads]() {
				for (size_t i = t; i < sections.size() && !result; i += numThreads) {
					const auto& sec = sections[i];
					if (!sec.Valid())
						continue;

					if (void* hit = ScanRange(sec.base, sec.size)) {
						void* expected = nullptr;
						result.compare_exchange_strong(expected, hit);
						return;
					}
				}
			});
		}

		// Wait for all threads
		for (auto& thread : threads) {
			thread.join();
		}

		if (!result) {
			Log("Didn't find signature '%s'\n", m_pattern.c_str());
		}

		return result;
	}

	void* Signature::ScanXref(const uint8_t* base, size_t size) const {
		uint8_t* hit = (uint8_t*)Scan(base, size);
		if (!hit)
			return nullptr;

		// x86 CALL / JMP
		if (hit[0] == 0xE8 || hit[0] == 0xE9) {
			int32_t rel = *(int32_t*)(hit + 1);
			return hit + 5 + rel;
		}

		return hit;
	}

	// возвращает true если найден displacement
	// outDisp — signed (может быть отрицательным)
	static bool ResolveDispFromInstr(const uint8_t* code, int32_t& outDisp) {
		if (!code) return false;

		size_t i = 0;

		// === 1. Skip REX prefixes ===
		if ((code[i] & 0xF0) == 0x40)
			i++;

		// === 2. Opcode ===
		uint8_t opcode = code[i++];

		// skip 0F xx
		if (opcode == 0x0F)
			opcode = code[i++];

		// === 3. ModRM ===
		uint8_t modrm = code[i++];
		uint8_t mod = (modrm >> 6) & 3;
		uint8_t rm = modrm & 7;

		bool hasSib = (rm == 4 && mod != 3);

		if (hasSib) {
			uint8_t sib = code[i++];
			rm = sib & 7; // base
		}

		// === 4. displacement ===
		if (mod == 1) { // disp8
			outDisp = (int8_t)code[i];
			return true;
		}
		if (mod == 2) { // disp32
			outDisp = *(int32_t*)(code + i);
			return true;
		}
		if (mod == 0 && rm == 5) { // RIP+disp32 or [disp32]
			outDisp = *(int32_t*)(code + i);
			return true;
		}

		return false;
	}

	int32_t Signature::FindDisp(const uint8_t* base, size_t size) const {
		auto p = (uint8_t*)Scan(base, size);
		if (!p) return 0;

		int32_t disp = 0;
		if (ResolveDispFromInstr(p, disp)) {
			if (!disp)
				Log("[FindDisp] '%s' NOT FOUND\n", m_pattern.c_str());
			else
				Log("[FindDisp] '%s' = 0x%X\n", m_pattern.c_str(), disp);

			return disp;
		}

		Log("FindDisp failed for '%s'\n", m_pattern.c_str());
		return 0;
	}
}
