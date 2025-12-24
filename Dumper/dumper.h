// dumper.h
#pragma once
#include "common.h"
#include "il2cpp_bridge.h"

namespace Dumper {
    // Generates structs.h for C++ SDK
    void GenerateSDK();

    // Dumps detailed info about all classes to log/console
    void DumpFull();
}