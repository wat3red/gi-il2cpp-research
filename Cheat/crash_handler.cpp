#include "crash_handler.h"
#include "logger/logger.h"

#include <dbghelp.h>
#include <psapi.h>
#include <stdio.h>

#pragma comment(lib, "dbghelp.lib")

namespace {

LONG g_in_crash_handler = 0;

bool ShouldLog(DWORD code) {
	switch (code) {
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_STACK_OVERFLOW:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_PRIV_INSTRUCTION:
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	case 0xE06D7363: // C++ SEH wrapper
		return true;
	default:
		return false;
	}
}

const char* ExceptionName(DWORD code) {
	switch (code) {
	case EXCEPTION_ACCESS_VIOLATION:      return "ACCESS_VIOLATION";
	case EXCEPTION_ILLEGAL_INSTRUCTION:   return "ILLEGAL_INSTRUCTION";
	case EXCEPTION_STACK_OVERFLOW:        return "STACK_OVERFLOW";
	case EXCEPTION_INT_DIVIDE_BY_ZERO:    return "INT_DIVIDE_BY_ZERO";
	case EXCEPTION_PRIV_INSTRUCTION:      return "PRIV_INSTRUCTION";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "ARRAY_BOUNDS_EXCEEDED";
	case 0xE06D7363:                      return "CPP_EXCEPTION";
	default:                              return "UNKNOWN";
	}
}

// Resolve VA to "Module.dll+0xOFF [symbol+disp] (file:line)".
// IL2CPP rarely has symbols, so module+offset is the useful part.
void ResolveAddress(uintptr_t addr, char* out, size_t out_size) {
	HMODULE module = nullptr;
	GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<LPCSTR>(addr), &module);

	char mod_name[MAX_PATH] = "<unknown>";
	if (module)
		GetModuleBaseNameA(GetCurrentProcess(), module, mod_name, sizeof(mod_name));

	const uintptr_t offset = module ? (addr - reinterpret_cast<uintptr_t>(module)) : addr;

	constexpr size_t kSymInfoSize = sizeof(SYMBOL_INFO) + MAX_SYM_NAME;
	alignas(SYMBOL_INFO) char sym_buf[kSymInfoSize]{};
	auto* sym = reinterpret_cast<SYMBOL_INFO*>(sym_buf);
	sym->SizeOfStruct = sizeof(SYMBOL_INFO);
	sym->MaxNameLen = MAX_SYM_NAME;

	DWORD64 disp64 = 0;
	DWORD disp_line = 0;
	IMAGEHLP_LINE64 line{};
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	const bool has_symbol = SymFromAddr(GetCurrentProcess(), addr, &disp64, sym) != 0;
	const bool has_line = has_symbol &&
		SymGetLineFromAddr64(GetCurrentProcess(), addr, &disp_line, &line) != 0;

	if (has_line) {
		_snprintf_s(out, out_size, _TRUNCATE,
			"%s+0x%llX  [%s+0x%llX]  (%s:%lu)",
			mod_name, static_cast<unsigned long long>(offset),
			sym->Name, static_cast<unsigned long long>(disp64),
			line.FileName, line.LineNumber);
	}
	else if (has_symbol) {
		_snprintf_s(out, out_size, _TRUNCATE,
			"%s+0x%llX  [%s+0x%llX]",
			mod_name, static_cast<unsigned long long>(offset),
			sym->Name, static_cast<unsigned long long>(disp64));
	}
	else {
		_snprintf_s(out, out_size, _TRUNCATE,
			"%s+0x%llX",
			mod_name, static_cast<unsigned long long>(offset));
	}
}

LONG WINAPI VectoredHandler(PEXCEPTION_POINTERS info) {
	const DWORD code = info->ExceptionRecord->ExceptionCode;
	if (!ShouldLog(code))
		return EXCEPTION_CONTINUE_SEARCH;

	// Re-entrancy guard (stack overflow inside this handler, etc.).
	if (InterlockedCompareExchange(&g_in_crash_handler, 1, 0) != 0)
		return EXCEPTION_CONTINUE_SEARCH;

	CONTEXT ctx = *info->ContextRecord;

	STACKFRAME64 frame{};
	frame.AddrPC.Offset = ctx.Rip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = ctx.Rbp;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = ctx.Rsp;
	frame.AddrStack.Mode = AddrModeFlat;

	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();

	// RIP=0 recovery: null-call. Real return address is at [RSP].
	if (frame.AddrPC.Offset == 0 && ctx.Rsp != 0) {
		uintptr_t ret_addr = 0;
		SIZE_T read = 0;
		if (ReadProcessMemory(process, reinterpret_cast<LPCVOID>(ctx.Rsp),
				&ret_addr, sizeof(ret_addr), &read)
			&& read == sizeof(ret_addr) && ret_addr != 0) {
			char resolved[512];
			ResolveAddress(ret_addr, resolved, sizeof(resolved));
			Log("  #0   0x%016llX  %s  <-- CALLER of null ptr\n",
				static_cast<unsigned long long>(ret_addr), resolved);

			ctx.Rip = ret_addr;
			ctx.Rsp += sizeof(uintptr_t);
			frame.AddrPC.Offset = ret_addr;
			frame.AddrStack.Offset = ctx.Rsp;
		}
	}

	// Walk once to decide if the crash is ours, then walk again for the log.
	constexpr int kMaxFrames = 62;
	char resolved[512];
	bool ours = false;
	{
		CONTEXT probe = ctx;
		STACKFRAME64 probe_frame = frame;
		for (int i = 1; i <= kMaxFrames; ++i) {
			if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread,
					&probe_frame, &probe, nullptr,
					SymFunctionTableAccess64, SymGetModuleBase64, nullptr)
				|| probe_frame.AddrPC.Offset == 0)
				break;

			ResolveAddress(static_cast<uintptr_t>(probe_frame.AddrPC.Offset),
				resolved, sizeof(resolved));
			if (strstr(resolved, "Cheat.dll"))
				ours = true;
		}
	}

	if (!ours) {
		InterlockedExchange(&g_in_crash_handler, 0);
		return EXCEPTION_CONTINUE_SEARCH;
	}

	Log("========== CRASH ==========\n");
	Log("Exception : 0x%08X (%s)\n", code, ExceptionName(code));
	Log("Address   : 0x%p\n", info->ExceptionRecord->ExceptionAddress);

	if (code == EXCEPTION_ACCESS_VIOLATION && info->ExceptionRecord->NumberParameters >= 2) {
		const char* op = info->ExceptionRecord->ExceptionInformation[0] == 1 ? "write" : "read";
		Log("AV Detail : %s at 0x%p\n",
			op, reinterpret_cast<void*>(info->ExceptionRecord->ExceptionInformation[1]));
	}

	Log("Registers :\n");
	Log("  RAX=%016llX  RBX=%016llX  RCX=%016llX  RDX=%016llX\n",
		ctx.Rax, ctx.Rbx, ctx.Rcx, ctx.Rdx);
	Log("  RSI=%016llX  RDI=%016llX  RBP=%016llX  RSP=%016llX\n",
		ctx.Rsi, ctx.Rdi, ctx.Rbp, ctx.Rsp);
	Log("  R8 =%016llX  R9 =%016llX  R10=%016llX  R11=%016llX\n",
		ctx.R8, ctx.R9, ctx.R10, ctx.R11);
	Log("  R12=%016llX  R13=%016llX  R14=%016llX  R15=%016llX\n",
		ctx.R12, ctx.R13, ctx.R14, ctx.R15);
	Log("  RIP=%016llX  EFL=%08X\n", ctx.Rip, ctx.EFlags);

	Log("Stack trace:\n");
	for (int i = 1; i <= kMaxFrames; ++i) {
		if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread,
				&frame, &ctx, nullptr,
				SymFunctionTableAccess64, SymGetModuleBase64, nullptr)
			|| frame.AddrPC.Offset == 0)
			break;

		ResolveAddress(static_cast<uintptr_t>(frame.AddrPC.Offset),
			resolved, sizeof(resolved));
		Log("  #%-2d  0x%016llX  %s\n",
			i, static_cast<unsigned long long>(frame.AddrPC.Offset), resolved);
	}
	Log("===========================\n");

	InterlockedExchange(&g_in_crash_handler, 0);
	return EXCEPTION_CONTINUE_SEARCH;
}

void* g_handler = nullptr;

} // namespace

void InstallCrashHandler() {
	SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
	SymInitialize(GetCurrentProcess(),
		"srv*C:\\symbols*https://msdl.microsoft.com/download/symbols",
		TRUE);
	g_handler = AddVectoredExceptionHandler(1, VectoredHandler);
}

void RemoveCrashHandler() {
	if (g_handler) {
		RemoveVectoredExceptionHandler(g_handler);
		g_handler = nullptr;
	}
	SymCleanup(GetCurrentProcess());
}
