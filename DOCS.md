# ARWN — Technical Reference Manual

*ALRI Web Native Compiler, Zero-Disk Container Runtime & WASM Engine*

*Version: 0.2.01 | Engineered by ALRI Development | Governed by ALRI GROUP © 2026 | License: ARGLP*

---

## Table of Contents

- [1. Overview & Architectural Philosophy](#1-overview--architectural-philosophy)
- [2. Architecture & Compilation Pipeline](#2-architecture--compilation-pipeline)
- [3. The .arweb Binary Container Format](#3-the-arweb-binary-container-format)
- [4. Configuration Reference (config.arwn)](#4-configuration-reference-configarwn)
- [5. Module Reference](#5-module-reference)
  - [5.1 Core Application (arwn_core)](#51-core-application-arwn_core)
  - [5.2 Builder Engine (arwn_builder)](#52-builder-engine-arwn_builder)
  - [5.3 Container Packer & CRC32 (arwn_pack)](#53-container-packer--crc32-arwn_pack)
  - [5.4 Base64 VM Obfuscator (arwn_obfuscator)](#54-base64-vm-obfuscator-arwn_obfuscator)
  - [5.5 Embedded HTTP Server (arwn_server & arwn_http)](#55-embedded-http-server-arwn_server--arwn_http)
  - [5.6 Gateway Auto-Registration (arwn_gateway)](#56-gateway-auto-registration-arwn_gateway)
  - [5.7 Declarative Config Parser (arwn_config)](#57-declarative-config-parser-arwn_config)
- [6. WebAssembly (WASM) Integration](#6-webassembly-wasm-integration)
- [7. Obfuscation & Intellectual Property Protection](#7-obfuscation--intellectual-property-protection)
- [8. CLI Reference (arwn_build)](#8-cli-reference-arwn_build)
- [9. Build & Packaging](#9-build--packaging)

---

## 1. Overview & Architectural Philosophy

**ARWN (ALRI Web Native)** is the proprietary web application packaging and execution framework for the ALRIOS platform. It eliminates traditional web server bottlenecks (runtime file lookups, dynamic bundling, unencrypted client assets) by compiling web interfaces and WASM logic into sealed, tamper-proof, in-memory binary containers (`.arweb`).

### Core Value Propositions

1. **Zero Runtime Disk Lookups**: Web applications are bundled into linear binary containers mounted directly into RAM. All HTTP requests are served from memory pointers without `open()`, `read()`, or `stat()` system call overhead.
2. **Deterministic Data Integrity**: Every section inside an `.arweb` container is individually checked against a hardware CRC32 checksum before mounting.
3. **Intellectual Property Shield**: Native self-executing Base64 VM obfuscation wraps frontend logic and injects sovereign ALRI Group copyright assertions.
4. **WASM Micro-Engines**: Linear memory execution of C, C++, and Rust algorithms alongside JavaScript.
5. **Autonomic Mesh Discovery**: When booted, ARWN services dynamically announce and register their virtual hosts and routing tables with the ARWS reverse proxy gateway via port 9500.

---

## 2. Architecture & Compilation Pipeline

```
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                         ARWN Two-Phase Lifecycle                            │
 │                                                                             │
 │  PHASE 1: BUILD & PACK (Compile Time)                                       │
 │  Source Assets (web/main.arhtml, main.js, main.css, calc.c)                 │
 │       │                                                                     │
 │       ▼                                                                     │
 │  arwn_builder (Compiles WASM, bundles JS/CSS, applies Base64 VM)            │
 │       │                                                                     │
 │       ▼                                                                     │
 │  arwn_pack (Builds .arweb binary, adds ALRIGROUP@ARWEB magic + CRC32)       │
 │       │                                                                     │
 │       ▼                                                                     │
 │  Output: build/<unit_name>.arweb                                            │
 │                                                                             │
 │ ─────────────────────────────────────────────────────────────────────────── │
 │                                                                             │
 │  PHASE 2: MOUNT & SERVE (Runtime)                                           │
 │  arwn_mount (Loads .arweb into linear RAM, zero-copy section indexing)      │
 │       │                                                                     │
 │       ├─────────────────────────────────┐                                   │
 │       ▼                                 ▼                                   │
 │  arwn_gateway                   arwn_server                                 │
 │  (Registers route on            (HTTP Event Loop on port 3055)              │
 │   ARWS gateway:9500)                    │                                   │
 │                                         ▼                                   │
 │                               Zero-Disk Memory Serve                        │
 └─────────────────────────────────────────────────────────────────────────────┘
```

---

## 3. The .arweb Binary Container Format

### Binary Memory Layout

```
┌──────────────────────────────────────────────────────────┐
│ Magic: "ALRIGROUP@ARWEB\0" (16 Bytes)                    │
├──────────────────────────────────────────────────────────┤
│ Version: uint32 (1) | Section Count: uint32              │
├──────────────────────────────────────────────────────────┤
│ Section Table (48 Bytes per Section):                    │
│   - Name (32 Bytes NULL-terminated)                      │
│   - Offset from start of data (uint32)                   │
│   - Size in bytes (uint32)                               │
│   - Section CRC32 Checksum (uint32)                      │
│   - Reserved (4 Bytes padding)                           │
├──────────────────────────────────────────────────────────┤
│ Data Payloads (Continuous Memory Buffer):                │
│   [Section 0: main.arhtml] (Raw Entrypoint HTML)         │
│   [Section 1: main.js]     (Obfuscated Base64 VM JS)     │
│   [Section 2: main.css]    (Compiled Minified Styles)    │
│   [Section 3: calc.wasm]   (WASM Linear Memory Engine)   │
└──────────────────────────────────────────────────────────┘
```

### Constraints & Constants

```c
#define ARWN_ARWEB_MAGIC         "ALRIGROUP@ARWEB"
#define ARWN_ARWEB_VERSION       1
#define ARWN_ARWEB_MAX_SECTIONS  64
#define ARWN_ARWEB_NAME_MAX      31
#define ARWN_PACK_MAX_PAYLOAD    (256 * 1024 * 1024) /* 256 MiB ceiling */
```

---

## 4. Configuration Reference (config.arwn)

The declarative configuration file governing packaging, routing, and compilation.

### Sample Configuration

```ini
[app]
name=my-app
port=3055
bind=127.0.0.1
copyright=Copyright (c) 2026 ALRI GROUP. All rights reserved.

[arws]
gateway=127.0.0.1:9500
route.host=myapp.localhost
route.path=/*
route.mode=production

[unit:main]
source=web/
entry=main.arhtml
compile=main.js
compile.lang=js
obfuscate=yes
copyright=Proprietary Frontend Core - Unauthorized copying prohibited.

[unit:calc]
source=units/c
compile=calc.wasm
compile.lang=c
obfuscate=no
```

### Options Specification

| Section | Key | Type | Description |
|---|---|---|---|
| `[app]` | `name` | string | Application identifier |
| `[app]` | `port` | int | Internal runtime TCP listen port |
| `[app]` | `bind` | string | Interface bind IP (typically `127.0.0.1`) |
| `[app]` | `copyright` | string | Global legal disclaimer injected into all outputs |
| `[arws]` | `gateway` | host:port | ARWS master gateway IPC control address |
| `[arws]` | `route.host` | string | Virtual host domain for reverse proxy matching |
| `[arws]` | `route.path` | string | Path pattern for reverse proxy (`/*`) |
| `[arws]` | `route.mode` | string | Operational target (`production`, `test`) |
| `[unit:<name>]` | `source` | path | Relative path to source asset directory |
| `[unit:<name>]` | `entry` | string | Primary entrypoint file (must be `main.arhtml`) |
| `[unit:<name>]` | `compile` | string | Target artifact name (`main.js`, `calc.wasm`) |
| `[unit:<name>]` | `compile.lang` | string | Compilation toolchain: `js`, `c`, `cpp`, `rust` |
| `[unit:<name>]` | `obfuscate` | bool | `yes` enables self-executing Base64 VM wrapper |
| `[unit:<name>]` | `copyright` | string | Unit-specific proprietary copyright header |

---

## 5. Module Reference

### 5.1 Core Application (arwn_core)

**Files**: `include/arwn.h`, `core/arwn_core.c`

**Purpose**: App context manager. Allocates and frees `arwn_app_t`, coordinates configuration parsing, builder invocation, and server mounting.

#### Key Functions

| Signature | Description |
|---|---|
| `arwn_app_t* arwn_app_new(const char *name)` | Allocate and initialize an ARWN application state |
| `void arwn_app_free(arwn_app_t *app)` | Clean up all unit memory, parsed configs, and buffers |
| `int arwn_mount(arwn_app_t *app)` | Build, pack, load `.arweb` into RAM, announce routes, and serve |

---

### 5.2 Builder Engine (arwn_builder)

**Files**: `core/arwn_builder.h`, `core/arwn_builder.c`

**Purpose**: Executes Phase 1 compilation across declared units:
1. Validates entrypoint naming rules (enforces `main.arhtml`, `main.js`, `main.css`).
2. Invokes language compilers (e.g., `emcc` or `clang` for WASM micro-units).
3. Invokes `arwn_obfuscator` for JavaScript when `obfuscate=yes`.
4. Injects ALRI Group and developer copyright headers.
5. Invokes `arwn_pack` to generate `.arweb` output files.

---

### 5.3 Container Packer & CRC32 (arwn_pack)

**Files**: `core/arwn_pack.h`, `core/arwn_pack.c`

**Purpose**: Low-level binary serialization and zero-copy indexing of `.arweb` files.

#### Structs

```c
typedef struct {
    char name[ARWN_ARWEB_NAME_MAX + 1];
    const void *data;
    uint32_t size;
} arwn_pack_section_t;
```

#### Functions

| Signature | Description |
|---|---|
| `int arwn_pack_build(const arwn_pack_section_t *sections, int count, uint8_t *out, size_t out_cap, size_t *out_len)` | Serializes sections, computes CRC32, writes container header |
| `int arwn_pack_validate(const uint8_t *data, size_t len)` | Verifies magic, section offsets, and re-computes CRC32 checksums |
| `int arwn_pack_index(const uint8_t *data, size_t len, arwn_pack_section_t *views, int views_cap)` | Returns zero-copy slice pointers directly into loaded memory buffer |
| `uint32_t arwn_crc32(const void *data, size_t len)` | High-speed CRC32 implementation |

---

### 5.4 Base64 VM Obfuscator (arwn_obfuscator)

**Files**: `core/arwn_obfuscator.h`, `core/arwn_obfuscator.c`

**Purpose**: Protects intellectual property by transforming source JavaScript into self-executing Base64 bytecode wrappers:

```javascript
/* (c) 2026 ALRI GROUP. Proprietary Core. */
(function(){
    var _0x=['VGhpcyBpcyBjb25maWRlbnRpYWw...'];
    var _d=atob(_0x[0]);
    new Function(_d)();
})();
```

---

### 5.5 Embedded HTTP Server (arwn_server & arwn_http)

**Files**: `core/arwn_server.h`, `core/arwn_server.c`, `core/arwn_http.c`

**Purpose**: Micro-server serving `.arweb` sections from linear memory pointers. Emits optimized headers:
- `Content-Type`: Automatically derived from section name
- `Content-Security-Policy`: Default strict sandbox
- `Cache-Control`: In-memory immutable asset caching

---

### 5.6 Gateway Auto-Registration (arwn_gateway)

**Files**: `core/arwn_gateway.h`, `core/arwn_gateway.c`

**Purpose**: On boot, opens a non-blocking TCP socket to ARWS port 9500, sending an `IPC_REGISTER` message:

```
IPC_REGISTER <app_name> /* GET <route.host> production proxy=http://127.0.0.1:<port>
```

Ensures zero-touch configuration: booting an ARWN app immediately makes it live across the global reverse proxy.

---

### 5.7 Declarative Config Parser (arwn_config)

**Files**: `core/arwn_config.h`, `core/arwn_config.c`

**Purpose**: Robust INI/ARWN dialect parser enforcing strict buffer safety limits (`ARWN_CFG_MAX_FILE = 64KB`, `ARWN_CFG_MAX_KEYS = 256`).

---

## 6. WebAssembly (WASM) Integration

ARWN applications can embed WASM micro-engines directly into the container section table. The included runtime bridge `runtime/arwn-bridge.js` exposes two-way communication:

```javascript
// arwn-bridge.js client usage
const bridge = await ARWN.loadModule('calc.wasm');
const result = bridge.instance.exports.compute(100000);
```

Supported languages for WASM units:
- **C** (compiled via `emcc` / `clang --target=wasm32`)
- **C++**
- **Rust** (`wasm-pack` / `cargo build --target wasm32-unknown-unknown`)
- **Go** (`GOOS=js GOARCH=wasm`)

---

## 7. Obfuscation & Intellectual Property Protection

When `obfuscate=yes` is set on a unit:
1. The builder reads the compiled JavaScript output.
2. The code is compressed and encoded into Base64 blocks.
3. A polyfill-safe self-invoking loader is wrapped around the payload.
4. Mandatory legal assertions from ALRI Group and the application author are prepended as immutable file comments.

---

## 8. CLI Reference (arwn_build)

```bash
# Build .arweb containers from config.arwn in current directory
arwn_build build

# Build to explicit output directory
arwn_build build -o /tmp/output/

# Mount and start serving immediately
arwn_build mount

# Inspect sections and CRC32 of an existing .arweb file
arwn_build info build/main.arweb

# Validate binary container integrity
arwn_build verify build/main.arweb
```

---

## 9. Build & Packaging

### Compilation

```bash
gcc -O2 \
  -I. -Iinclude -Icore \
  -I../../ALRIOS/arkernel/include \
  -I../../ALRIOS/arkernel/os/include \
  -o $STAGING/arwn_build \
  tools/arwn_build.c core/arwn_core.c core/arwn_server.c \
  core/arwn_http.c core/arwn_gateway.c core/arwn_builder.c \
  core/arwn_obfuscator.c core/arwn_pack.c core/arwn_config.c \
  -L$ARCORE/lib -larkernel -lssl -lcrypto
```

### .arapp Manifest (`arwn.arappmake`)

```json
{
  "name": "arwn",
  "version": "0.2.01",
  "runtime": "native",
  "entry": "arwn_build",
  "files": ["arwn_build", "arwn.h"],
  "description": "Runtime Nativo Web e Servidor de Containers .arweb",
  "commands": ["status", "routes", "ping"]
}
```

---

*Document generated from source code analysis of ARWN v0.2.01.*
*Engineered by ALRI Development. Governed by ALRI GROUP © 2026 — All rights reserved.*
*License: ARGLP (ALRI GROUP LICENSE PERMISSIVE — Version 2)*
