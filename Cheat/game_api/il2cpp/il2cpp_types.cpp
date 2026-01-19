#include "il2cpp_types.h"
#include "../functions/resolve_funcs.h"

#include <string.h>

Il2CppString* Il2CppString::FromCStr(const char* c_str)
{
	return il2cpp_string_new_len(c_str, strlen(c_str));
}

const char* Il2CppString::ToCStr()
{
	return Il2Cpp::Method::Call<const char*>("System.Runtime.InteropServices", "Marshal", "StringToHGlobalAnsi", 1, this);
}
