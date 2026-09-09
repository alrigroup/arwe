# AGENTS.md — Autonomous AI Agent Operating Protocol & Technical Invariants

> **Target Audience**: Autonomous AI Agents (Antigravity, Claude Code, Cursor, Copilot) & Compiler/Runtime Engineers.  
> **Repository**: `arwe` (ALRI Web Engine — Container Runtime & WASM Engine)  
> **Visibility**: Public Open-Core  
> **Asset Owner**: ALRI Group | **Engineering**: ALRI Development  
> **License**: ARGLP (ALRI Group License Permissive — Version 2)  
> **Primary Technical Reference**: Consult [`DOCS.md`](DOCS.md) for binary memory layout specifications, section structures, and bridge APIs.

---

## 1. Project Mission & Identity

**ARWE (ALRI Web Engine)** is the application packaging, compilation toolchain, and runtime framework for the ALRIOS platform. It eliminates traditional web server file-lookup overhead by compiling frontend web interfaces and WebAssembly micro-units into sealed, tamper-proof, binary containers (`.arweb`) served directly from shared RAM without runtime disk I/O.

### Core Architectural Specifications
- **Binary Container Format (`.arweb`)**:
  - Magic Header: `ALRIGROUP@ARWEB\0` (16 bytes).
  - Version: `uint32_t` (1).
  - Section Descriptor Table: 48 bytes per section (`name[32]`, `offset[uint32]`, `size[uint32]`, `crc32[uint32]`, `reserved[uint32]`).
  - Maximum Payload Ceiling: `256 MiB` (`ARWE_PACK_MAX_PAYLOAD`).
- **Intellectual Property Shield**: Native self-executing Base64 VM obfuscation (`arwe_obfuscator.c`) wrapping JavaScript into protected runtime containers when `obfuscate=yes`.
- **WASM Bridge**: Bidirectional linear memory execution of C, C++, Rust, and Go micro-units via `runtime/arwe-engine.js`.
- **Zero-Touch Routing**: Automatically announces and registers virtual host routes with the ARWS reverse proxy on TCP port `9500` upon boot.

---

## 2. Directory Structure & Key Files

```
src-arapps/arwe/
├── arwe.arappmake           # ALRIOS package manifest & compilation rules
├── DOCS.md                   # Complete 360+ lines technical reference manual
├── README.md                 # Public overview & operational guide
├── AGENTS.md                 # This autonomous agent operating protocol
├── include/
│   ├── arwe.h                # Public C API declarations, unit structs, parser limits
│   └── arwe.hpp              # C++ namespace wrappers
├── core/
│   ├── arwe_core.c           # App lifecycle, allocation and mount coordinator
│   ├── arwe_builder.c        # Phase 1: compiler orchestrator, asset bundler
│   ├── arwe_pack.c           # Binary container packer, CRC32 serializer, zero-copy indexer
│   ├── arwe_pack.h           # Container structs (arwe_pack_section_t) and packing limits
│   ├── arwe_obfuscator.c     # Base64 VM JavaScript wrapper & copyright injector
│   ├── arwe_server.c         # Embedded HTTP server serving .arweb slices from linear RAM
│   ├── arwe_http.c           # HTTP 1.1 request parser and header emitter
│   ├── arwe_gateway.c        # IPC client auto-registering routes on ARWS (port 9500)
│   └── arwe_config.c         # INI parser enforcing strict anti-bomb buffer safety limits
├── tools/
│   └── arwe_build.c          # Central CLI binary (build, mount, info, verify commands)
└── runtime/
    ├── arwe-engine.js        # JavaScript runtime bridge for loading and executing WASM
    └── arwe.d.ts             # TypeScript definitions for the ARWE browser runtime
```

---

## 3. Essential Commands & Toolchain Invariants

### 3.1 Compilation of the `arwe_build` Toolchain
```bash
gcc -O2 \
  -I. -Iinclude -Icore -I../../ALRIOS/arkernel/include -I../../ALRIOS/arkernel/os/include \
  -o arwe_build \
  tools/arwe_build.c core/arwe_core.c core/arwe_server.c core/arwe_http.c \
  core/arwe_gateway.c core/arwe_builder.c core/arwe_obfuscator.c core/arwe_pack.c core/arwe_config.c \
  -L../../ALRIOS/arcore/lib -larkernel -lssl -lcrypto
```

### 3.2 Building `.arweb` Containers from `config.arwe`
```bash
./arwe_build build              # Parses config.arwe, compiles units to build/<unit>.arweb
./arwe_build build -o /tmp/out/ # Builds containers into explicit destination directory
```

### 3.3 Inspecting and Verifying Binary Containers
```bash
./arwe_build info <app.arweb>   # Dumps magic, version, section names, offsets, CRC32s
./arwe_build verify <app.arweb> # Validates header integrity, boundary clamps, and CRC32 digests
```

### 3.4 Mounting and Serving Live
```bash
./arwe_build mount              # Builds on the fly, loads container into RAM, serves HTTP
```

---

## 4. Architectural Rules & The "NEVER" List

Autonomous AI Agents operating within this codebase must strictly observe these inviolable rules:

### 4.1 Strict Prohibitions
- ❌ **NEVER violate the "MAIN" naming convention**: Inside any web unit, assets must be strictly named `main.arhtml`, `main.js`, and `main.css`. Arbitrary names like `index.js` or `styles.css` will break container resolution.
- ❌ **NEVER allow inline `<script>` or `<style>` tags in `main.arhtml`**: All logic and styling must be segregated into `main.js` and `main.css` to allow the compiler to apply obfuscation and minification.
- ❌ **NEVER read files from disk during request handling**: The `arwe_server` must serve payloads strictly using memory pointers obtained via `arwe_pack_index()`. Zero runtime disk calls are permitted.
- ❌ **NEVER edit `.arweb` files with manual string replacement**: Containers must be serialized through `arwe_pack_build()` to maintain valid CRC32 digests and 48-byte section boundaries.
- ❌ **NEVER exceed the anti-bomb parser bounds**: Configuration parsing enforces strict maximums (`ARWE_CFG_MAX_FILE = 64KB`, `ARWE_CFG_MAX_KEYS = 256`, `ARWE_CFG_MAX_SECTIONS = 16`).

---

## 5. Code Style & Engineering Standards

### 5.1 Correct vs. Incorrect Implementations

#### Container Section Registration
```c
/* INCORRECT: Manually manipulating container offsets or bypassing CRC32 */
memcpy(out + offset, payload, payload_size);

/* CORRECT (ARWE Standard): Structured section definition with hardware CRC32 */
arwe_pack_section_t sections[4];
strncpy(sections[0].name, "main.arhtml", ARWE_ARWEB_NAME_MAX);
sections[0].data = html_buffer;
sections[0].size = (uint32_t)html_len;

size_t out_len = 0;
int res = arwe_pack_build(sections, section_count, out_buf, out_capacity, &out_len);
if (res < 0) {
    // Handle container build error gracefully
}
```

#### Zero-Copy In-Memory Serving
```c
/* CORRECT (ARWE Standard): Serving directly from memory views */
arwe_pack_section_t views[ARWE_ARWEB_MAX_SECTIONS];
int count = arwe_pack_index(container_data, container_len, views, ARWE_ARWEB_MAX_SECTIONS);
for (int i = 0; i < count; i++) {
    if (strcmp(views[i].name, requested_file) == 0) {
        arwe_http_send_response(conn, 200, views[i].name, views[i].data, views[i].size);
        return;
    }
}
```

### 5.2 Mandatory Copyright Header
Every new source or header file created must begin with:
```c
/*
 * Copyright (c) 2026 ALRIGROUP and its affiliates.
 * Engineered and maintained by ALRI Development.
 *
 * This code is licensed under the ARGLP - ALRI GROUP LICENSE PERMISSIVE
 * found in the LICENSE file in the root directory of this source tree
 * and at: https://github.com/alrigroup/licenses
 */
```

---

## 6. Pre-Commit & Pull Request Verification Checklist

Before submitting changes, the agent must verify:
1. `tools/arwe_build.c` and all `core/*.c` files compile cleanly with zero warnings under `-O2 -Wall -Wextra`.
2. Unit tests in `tests/test_core.c` pass completely.
3. The `.arweb` binary specification (magic string, 48-byte section header, CRC32) remains backward-compatible with version 1.
4. No `.arweb` binary files or build directories are committed to Git.
5. Git commits adhere to Conventional Commits with the mandatory trailer:
   `Signed-off-by: ALRI Development <dev@alrigroup.com>`.
