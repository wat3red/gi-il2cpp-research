#pragma once

#include "../il2cpp/il2cpp_types.h"
#include "../il2cpp/il2cpp.h"
#include <logger/logger.h>

#include <vector>
#include <utility>
#include <cassert>

namespace Unity
{
	template<typename T>
	struct Array : Il2CppObject {
		Il2CppArrayBounds* bounds;
		il2cpp_array_size_t max_length;
		T items[0];

		size_t length() const { return (bounds == nullptr) ? max_length : bounds->length; }

		T* begin() { return &items[0]; }
		T* end() { return &items[length()]; }
		const T* begin() const { return &items[0]; }
		const T* end() const { return &items[length()]; }

		T& operator[](int i) {
			return items[i];
		}

		const T& operator[](int i) const {
			return items[i];
		}

		std::vector<T> to_vector() const {
			std::vector<T> result;
			auto len = length();
			result.reserve(len);
			for (size_t i = 0; i < len; ++i)
				result.push_back(items[i]);
			return result;
		}
	};

	template<typename T>
	struct List : Il2CppObject {
		Array<T>* array;
		int32_t size;
		int32_t version;

		std::vector<T> to_vector() const {
			std::vector<T> result;
			if (!array || size <= 0) return result;
			result.reserve(size);
			for (int i = 0; i < size; i++) {
				result.push_back(array->items[i]);
			}
			return result;
		}
	};

	template<typename K, typename V>
	struct Dictionary : Il2CppObject {
		struct Entry {
			int32_t hashCode;
			int32_t next;
			K key;
			V value;
		};

		Array<Entry>* GetEntries() const {
			return *(Array<Entry>**)((uintptr_t)this + Il2Cpp::Field::GetOffsetFromName(klass, "entries"));
		}

		int32_t GetCount() const {
			return *(int32_t*)((uintptr_t)this + Il2Cpp::Field::GetOffsetFromName(klass, "count"));
		}

		std::vector<std::pair<K, V>> to_vector() const {
			std::vector<std::pair<K, V>> result;

			Array<Entry>* entries = GetEntries();

			if (!entries) return result;

			auto len = entries->length();
			result.reserve(len);

			for (int i = 0; i < len; i++) {
				const auto& entry = entries->items[i];

				if (i >= GetCount())
					break;

				if (entry.hashCode < 0)
					continue;

				result.emplace_back(entry.key, entry.value);
			}

			return result;
		}

		int FindEntry(K key) const {
			for (int i = 0; i < GetCount(); i++) {
				if ((*GetEntries())[i].key == key) return i;
			}
			return -1;
		}

		V operator[](K key) {
			int i = FindEntry(key);
			if (i >= 0) return (*GetEntries())[i].value;
			return V();
		}
	};
} // namespace Unity
