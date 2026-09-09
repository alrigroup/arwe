<p align="center">
  <img src="https://raw.githubusercontent.com/alrigroup/.github/main/alrigroup.svg" width="120" />
</p>

<h1 align="center">ARWE</h1>
<p align="center"><strong>ALRI Web Engine — Container Runtime & WASM Engine</strong></p>
<p align="center">
  <a href="https://github.com/alrigroup/alrios"><img alt="ALRIOS" src="https://img.shields.io/badge/Powered%20by-ALRIOS-blue?style=flat-square" /></a>
  <img alt="Language" src="https://img.shields.io/badge/language-C-00599C?style=flat-square" />
  <img alt="License" src="https://img.shields.io/badge/license-ARGLP-green?style=flat-square" />
</p>

---

## Overview

**ARWE** (ALRI Web Engine) is a compiler, bundler, and runtime that enables building web applications as native ALRIOS apps. It compiles `.arhtml` templates and JavaScript into optimized, deployable web packages.

### Features

- 🔨 **Compiler** — Compiles `.arhtml` + JS into optimized web bundles
- 📦 **Packager** — Creates `.arapp` packages for deployment via ALRIOS
- 🌐 **Gateway** — Built-in HTTP gateway for serving compiled web apps
- 🔒 **Obfuscator** — Optional code obfuscation for production builds
- 🧩 **Bridge API** — JavaScript bridge (`arwe-engine.js`) for native OS integration
- 📝 **TypeScript Support** — Ships with TypeScript definitions (`arwe.d.ts`)

## Building

```bash
armake build arwe
```

## Creating a Web App using ARWE

```bash
arcreate web myapp
cd myapp
arwe_build build
```

## Part of ALRIOS

ARWE is a core component of the [ALRIOS Operating System](https://github.com/alrigroup/alrios).

---

<p align="center">© 2026 ALRI Group — All rights reserved.</p>

---

## License

This project is licensed under the **ARGLP** (ALRI Group License Permissive) - see the [LICENSE-ARGLP](https://github.com/alrigroup/licenses/blob/main/LICENSE-ARGLP) file for full terms.

*Commercial and enterprise use is permitted. Resale of the software itself is prohibited.*

---

## 🏢 Credits & Governance

- **Engineering & Architecture**: **[ALRI Development](https://alrigroup.com)** *(Software & Systems Division)*
- **Holding & Asset Management**: **[ALRI Group](https://alrigroup.com)** *(Holding Company)*
- **Licensing**: Governed by **ARGLP** owned by ALRI Group.

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://cdn.alrigroup.com/ARD-SF-W.png">
    <source media="(prefers-color-scheme: light)" srcset="https://cdn.alrigroup.com/ARD-SF-B.png">
    <img alt="ARD Seal" src="https://cdn.alrigroup.com/ARD-SF-W.png" width="80">
  </picture><br>
  <sub>© 2026 ALRI Group and its affiliates. Engineered by ALRI Development.</sub>
</p>
