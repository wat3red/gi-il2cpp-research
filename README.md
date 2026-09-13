# GICheat

**Archived reverse-engineering case study** — an IL2CPP research toolkit built against Genshin Impact (Unity / IL2CPP), developed November 2025 → March 2026, frozen on game version **6.4**, and cleaned up afterwards as a portfolio artifact. [MIT](LICENSE).

This is **not** a usable product. Offsets and signatures are stale. Expect it not to run against a current client.

---

## What this actually is

Three Visual Studio projects that exercise the full path from “closed IL2CPP binary” to “typed C++ API + runtime overlay”:

| Project | Output | Role |
| --- | --- | --- |
| **Dumper** | `Dumper.dll` | Walks IL2CPP metadata at runtime, emits a C++ SDK and an IDA Pro import script |
| **Cheat** | `Cheat.dll` | In-process overlay: signature-resolved game API, DirectX 11 + ImGui menu, feature plugins |
| **Injector** | `Injector.exe` | Classic `CreateRemoteThread` + `LoadLibraryA` injector with a short-lived sandbox |

Shared utilities live in `Common/` and are compiled into both DLLs.

The interesting work is **not** the feature list. It is the layer underneath:

- resolving obfuscated IL2CPP methods without a static SDK
- keeping a working surface across several game patches (6.2 → 6.4)
- turning runtime metadata into something IDA can import
- wrapping the result in a small, typed feature framework

---

## Disclaimer

- For **educational / reverse-engineering portfolio** purposes only.
- Violates the game’s Terms of Service. Do **not** use this on a live account.
- No support, no updates, no guarantee it builds or does anything useful today.
- First-party code is MIT (see [LICENSE](LICENSE)). Vendored third-party libraries keep their own licenses.

---

## Architecture

```text
                    ┌─────────────────────┐
                    │   Injector.exe      │
                    │  LoadLibrary inject │
                    └──────────┬──────────┘
                               │
              ┌────────────────┼────────────────┐
              ▼                                 ▼
     ┌────────────────┐               ┌─────────────────┐
     │  Dumper.dll    │               │   Cheat.dll     │
     │                │               │                 │
     │ IL2CPP bridge  │               │ IL2CPP bridge   │
     │ (sig-scanned)  │               │ (sig-scanned)   │
     │      │         │               │       │         │
     │      ▼         │               │       ▼         │
     │ Metadata walk  │               │ Typed wrappers  │
     │      │         │               │ (MoleMole/Unity)│
     │      ▼         │               │       │         │
     │ SDK + IDA out  │               │ Feature plugins │
     └────────────────┘               │       │         │
                                      │       ▼         │
                                      │ DX11 Present    │
                                      │ hook + ImGui    │
                                      └─────────────────┘
                         ▲                     ▲
                         └────── Common/ ──────┘
                           network blocker, etc.
```

### Runtime function resolution

Game methods are not imported by name. They are resolved on boot via a small macro DSL in `Cheat/game_api/functions/functions_list.h`:

```cpp
// absolute RVA (fragile, dies every patch)
RESOLVE_BY_OFFSET(...)

// byte-pattern scan
RESOLVE_BY_SIGNATURE(...)

// find a call site, follow the relative call
RESOLVE_BY_XREF_SIGNATURE(...)
```

XREF scanning is the durable path: obfuscated names and shuffled RVAs change every patch, but the *shape* of a call sequence often does not. `Mem::Signature` in `Cheat/game_api/memory/` implements the pattern parser, SIMD scan, and the xref walk; `OffsetDB` memoizes resolved displacements by name.

A second, slower path uses the full IL2CPP API surface (also signature-resolved) to look up classes and methods at runtime:

```cpp
Il2Cpp::Method::Find("MiHoYo.SDK", "Dll", "Update", 0);
Il2Cpp::Method::Call<...>(klass, "name", argc, args...);
```

That is how the feature tick is hooked — by wrapping `MiHoYo.SDK.Dll.Update` with MinHook.

### Feature model

Features are plain classes with five optional hooks (`Cheat/features/feature.h`):

| Hook | Thread | Purpose |
| --- | --- | --- |
| `OnInit` | loader | install hooks, resolve pointers |
| `OnUpdate` | game | per-frame game logic |
| `DrawUI` | render | ImGui settings when menu is open |
| `DrawBackgroundUI` | render | overlay drawing (ESP etc.) |
| `UpdateHotkeys` | render | throttled to 100 ms |

Registry lives in `Cheat/features/features.cpp`. Config values are `ConfigVar<T>` wrappers that persist through JSON and bind directly to ImGui widgets.

### Dumper outputs

`Dumper` can emit three things (selected in `Dumper/dllmain.cpp`):

1. **Full metadata dump** — classes, methods, fields, params to log
2. **C++ SDK** — `structs.h`-style headers for offline use
3. **IDA import** — `ida.h` + `ida_import.py` + `ida_methods.json` so IDA gets names, types, and struct layouts

The IDA path walks the same IL2CPP graph the SDK path does, but sanitizes identifiers into valid C and expands generic/array type graphs (`Dumper/dump_for_ida.h`).

---

## Repository layout

```text
GICheat/
├── Common/                 # shared by both DLLs
│   └── network_blocker.*   # TCP-table telemetry killer
├── Cheat/                  # injected overlay DLL
│   ├── dllmain.cpp         # bootstrap only
│   ├── crash_handler.*     # vectored EH + stack walk (ours-only)
│   ├── directx_hook.cpp    # Present / ResizeBuffers hooks
│   ├── gui/                # ImGui menu shell
│   ├── features/           # one .cpp/.h pair per feature
│   ├── config/             # ConfigVar + JSON + ImGui binding
│   ├── game_api/
│   │   ├── il2cpp/         # runtime IL2CPP API + type headers
│   │   ├── memory/         # signature scanner + offset DB
│   │   ├── functions/      # RESOLVE_* macros + function table
│   │   ├── game/           # MoleMole wrappers
│   │   └── unity/          # Unity primitives
│   ├── logger/
│   └── external/           # vendored imgui, minhook, json.hpp
├── Dumper/                 # IL2CPP dumper DLL
├── Injector/               # standalone injector EXE
└── GICheat.slnx
```

First-party code is roughly **~12k lines** of C++ (excluding vendored third-party).

---

## Feature inventory

| Feature | Notes |
| --- | --- |
| ESP (entities, 2D box, name, snapline) | per-type config |
| Map teleport / waypoint walk | client-side path preferred |
| Custom teleport points | persisted list |
| Quest teleport | early |
| Noclip | camera-relative, kinematic |
| God mode | |
| Auto loot / auto talk | |
| Auto destroy | durability-drain hook |
| Skip cutscene | |
| Game speed | |
| Costume changer | avatar / costume / flycloak mapping |

---

## Build

Requires **Visual Studio 2022+** with the C++ desktop workload and a 64-bit Windows SDK. Open `GICheat.slnx`, build `x64 / Debug` (the only configuration ever used).

No package manager, no CI, no tests. Dependencies are vendored.

Paths are resolved at runtime:

- Injector looks for `Cheat.dll` next to itself, then `x64/Debug/Cheat.dll`
- Game path is auto-probed under common HoYoPlay install locations, or passed as `argv[2]`
- `start_cheat.bat` / `start_dumper.bat` use `%~dp0` so they work from any checkout

---

## Cleanup pass (post-freeze)

After freezing the research, the codebase was revised to remove the noise that accumulated during the reverse-engineering sprint. That pass is part of the case study:

**Structure**
- Extracted `NetworkBlocker` from both `dllmain.cpp` files into `Common/` (was a ~150-line copy-paste).
- Moved the vectored crash handler out of the Cheat bootstrap into `Cheat/crash_handler.*`.
- Slimmed both `dllmain.cpp` files down to actual bootstrap work.

**Deleted dead weight**
- Empty stub feature (`infinite_stamina`).
- `KillAura` — ~900 lines of half-applied hooks and raw RVA probes. Unsafe solution :(
- Disabled `NetworkAnalyzer` and its empty UI.
- `test_tu.cpp`, empty `TELEPORT_EXAMPLES/` tree.

**Kept on purpose (research trail)**
- Commented `RESOLVE_BY_*` lines in `functions_list.h` and class-anchor comments in `version_constants.h`. Each one records *how* the symbol was identified (dump regex, C# signature, previous RVA) and is reused when re-resolving after a patch.
- Inline “how to find this again” notes next to version-locked offsets in feature code.

---

## What I took away

- **XREF-relative call resolution** ages far better than absolute RVAs against a packed, per-patch IL2CPP binary.
- Dumping metadata at runtime and *re-importing it into IDA* turns every future patch from a RE session into a diff.
- A tiny `Feature` interface with explicit thread affinity (`OnUpdate` vs `DrawUI`) prevented a lot of ImGui/threading bugs.
- Process notes (dump regex, anchor signatures, previous RVAs) are documentation, not junk — keep them next to the symbols they identify.
- Feature code that is “kept just in case” and installs nothing should be deleted; the research lives on as one signature line plus a comment.
- Copy-pasting a helper into two projects is fine for a weekend spike and terrible three months later. Extract at the second use.
- Vendoring an entire library tree is convenient for research and noisy for a public repo.

---

## Timeline

| When | What |
| --- | --- |
| 2025-11 | first dumper, method/field walk |
| 2025-11 → 12 | dumper split from the cheat, ESP WIP |
| 2026-01 | Il2CPP API rewrite, SDK generator, IDA import; patch 6.2 / 6.3 |
| 2026-02 | feature expansion (map TP, costume changer, crash handling); patch 6.4 |
| 2026-03 | auto-destroy, custom/quest teleport; freeze |
| post-freeze | structural cleanup (this README’s “Cleanup pass”) |

---

## License

This repository is released under the [MIT License](LICENSE).

Third-party components vendored under `Cheat/external/` and `Dumper/lib/` keep their upstream licenses (MIT for Dear ImGui, MinHook, and nlohmann/json). The MIT grant above covers first-party code only; it does not grant rights to any game assets, trademarks, or services referenced by this research.
