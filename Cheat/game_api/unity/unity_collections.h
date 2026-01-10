#pragma once
#include "../il2cpp/il2cpp_types.h"

namespace Unity {

    template<typename T>
    struct Array : Il2CppObject {
        void* bounds;
        int max_length;
        T items[1];

        T& operator[](int i) { return items[i]; }
    };

    template<typename T>
    struct List : Il2CppObject {
        Array<T>* array;
        int size;
        int version;
    };

    template<typename K, typename V>
    struct Dictionary {
        struct Entry {
            int hashCode;
            int next;
            K key;
            V value;
        };

        Array<int>* buckets;
        Array<Entry>* entries;
        int count;
    };
}
