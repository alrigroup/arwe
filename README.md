<p align="center">
  <img src="https://raw.githubusercontent.com/alrigroup/.github/main/alrigroup.svg" width="120" />
</p>

<h1 align="center">ARWN</h1>
<p align="center"><strong>Web Native Compiler & Runtime</strong></p>
<p align="center">
  <a href="https://github.com/alrigroup/alrios"><img alt="ALRIOS" src="https://img.shields.io/badge/Powered%20by-ALRIOS-blue?style=flat-square" /></a>
  <img alt="Language" src="https://img.shields.io/badge/language-C-00599C?style=flat-square" />
  <img alt="License" src="https://img.shields.io/badge/license-ARGLP-green?style=flat-square" />
</p>

---

## Overview

**ARWN** (ALRI Web Native) is a compiler, bundler, and runtime that enables building web applications as native ALRIOS apps. It compiles `.arhtml` templates and JavaScript into optimized, deployable web packages.

### Features

- 🔨 **Compiler** — Compiles `.arhtml` + JS into optimized web bundles
- 📦 **Packager** — Creates `.arapp` packages for deployment via ALRIOS
- 🌐 **Gateway** — Built-in HTTP gateway for serving compiled web apps
- 🔒 **Obfuscator** — Optional code obfuscation for production builds
- 🧩 **Bridge API** — JavaScript bridge (`arwn-bridge.js`) for native OS integration
- 📝 **TypeScript Support** — Ships with TypeScript definitions (`arwn.d.ts`)

## Building

```bash
armake build arwn
```

## Creating a Web Native App

```bash
arcreate web myapp
cd myapp
arwn build
```

## Part of ALRIOS

ARWN is a core component of the [ALRIOS Operating System](https://github.com/alrigroup/alrios).

---

<p align="center">© 2025 ALRI Group — All rights reserved.</p>

---

## License

This project is licensed under the **ARGLP** (ALRI Group License Permissive) - see the [LICENSE-ARGLP](https://github.com/alrigroup/licenses/blob/main/LICENSE-ARGLP) file for full terms.

*Commercial and enterprise use is permitted. Resale of the software itself is prohibited.*

---

## 🏢 Credits & Governance

- **Engineering & Architecture**: **[ALRI Development](https://alrigroup.com)** *(Software & Systems Division)*
- **Holding & Asset Management**: **[ALRI Group](https://alrigroup.com)** *(Holding Company)*
- **Licensing**: Governed by **ARGLP** owned by ALRI Group.

<p align="center">© 2026 ALRI Group and its affiliates. Engineered by ALRI Development.</p>
