#include "signature.h"
#include "../utils.h"

#include <Windows.h>
#include <emmintrin.h>
#include <winnt.h>
#include <cstring>
#include <cctype>

namespace Mem {

	struct Section {
		uint8_t* base;
		size_t   size;

		bool Valid() const { return base && size; }
	};

	Section GetTextSection(HMODULE module)
	{
		if (!module)
			return {};

		auto dos = (PIMAGE_DOS_HEADER)module;
		if (dos->e_magic != IMAGE_DOS_SIGNATURE)
			return {};

		auto nt = (PIMAGE_NT_HEADERS)(
			(uint8_t*)module + dos->e_lfanew);

		if (nt->Signature != IMAGE_NT_SIGNATURE)
			return {};

		auto section = IMAGE_FIRST_SECTION(nt);

		for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section) {
			//printf("section->Name: %s\n", section->Name);
			if (strcmp((char*)section->Name, ".text") == 0) {
				return {
					(uint8_t*)module + section->VirtualAddress,
					section->Misc.VirtualSize
						? section->Misc.VirtualSize
						: section->SizeOfRawData
				};
			}
		}

		return {};
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
				m_bytes.push_back(
					static_cast<uint8_t>(
						std::stoi(pattern.substr(i, 2), nullptr, 16)));
				m_mask.push_back(0xFF);
				++i;
			}
		}
	}

	static inline bool match16(__m128i cmp, __m128i mask) {
		__m128i r = _mm_and_si128(cmp, mask);
		return _mm_movemask_epi8(r) == _mm_movemask_epi8(mask);
	}

	void* Signature::Scan(const uint8_t* base, size_t size) const {
		if (!Valid())
			return nullptr;

		if (!base || size < m_bytes.size()) {
			HMODULE game = GetModuleHandleW(nullptr); // main exe
			Section text = GetTextSection(game);
			if (!text.Valid())
				return nullptr;

			base = text.base;
			size = text.size;
		}

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

		const size_t maxSimd = (size >= 16) ? size - 16 : 0;
		const size_t scanEnd = std::min<size_t>(end, maxSimd);

		for (size_t i = 0; i <= scanEnd; ++i) {
			__m128i mem = _mm_loadu_si128((__m128i*)(base + i));
			__m128i cmp = _mm_cmpeq_epi8(mem, vpat);

			if (!match16(cmp, vmsk))
				continue;

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

		Utils::Log("Didn't find signature '%s'\n", m_pattern.c_str());

		return nullptr;
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

}
