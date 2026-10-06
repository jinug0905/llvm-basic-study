# llvm-basic-study

A hands-on practice repo for learning LLVM IR. Each lesson takes a small C program,
looks at the IR a compiler produces for it, then rebuilds that same IR by hand with
the LLVM C++ API (`IRBuilder`).

## Layout

```
llvm-basic-study/
└── src/
    ├── 0-hello-world/            printf, declarations vs. definitions, IRBuilder basics
    ├── 1-module-and-main/        a function that returns a constant; basic blocks and terminators
    ├── 2-variables-and-constants/  globals vs. locals (alloca), constant data
    ├── 3-types-and-constants/    integer types, casts (sext / trunc / sitofp / fptosi)
    ├── 4-operators/              arithmetic, comparisons, signed vs. unsigned opcodes
    ├── 5-branches/               if / else as a graph of basic blocks
    └── 6-loops/                  for / while as blocks with a backward edge
```

## File naming inside a lesson

| Pattern | What it is |
|---|---|
| `name.c` / `name.cpp` | The plain source program |
| `name.ll` | LLVM IR in text form for that program |
| `name_builder.cpp` | C++ program that builds the same IR with the LLVM API |
| `name_builder.ll` | IR printed by running the builder program |
| `*.s` | Assembly produced by `llc` or `clang -S` |
| `*.md` | Study notes (e.g. `0-hello-world/BUILDER_NOTES.md`) |

Each `*_builder.cpp` starts with a QUICK NOTES comment block summarizing the concepts
for that lesson. Later lessons point back to earlier ones instead of repeating them.

## Environment

### Hardware and OS

| Item | Value |
|---|---|
| Machine | Apple M3 Max, 16 cores, 64 GB RAM (Apple Silicon, arm64) |
| OS | macOS 26.6.2 (Darwin 25.6.0) when recorded |
| Xcode / SDK | Xcode 27.0, macOS SDK 27.0 |
| System compiler | Apple clang 21.0.0 (used only to build LLVM itself) |
| Build tools | CMake 4.1.0, Ninja 1.13.1 (Homebrew, `/opt/homebrew`) |
| Libraries | `zstd`, `zlib` (Homebrew), `libxml2` (macOS SDK) |
| Python | 3.9.13 (needed by LLVM's `lit` test runner) |

### LLVM toolchain (built from source, not installed)

| Item | Value |
|---|---|
| Source | `https://github.com/llvm/llvm-project.git`, branch `main` |
| Commit | `d26ea02060b1` (2025-08-21) |
| Version | `22.0.0git`, a development snapshot, **not** a release |
| Build | Ninja, `Release`, projects = `clang` only, no runtimes |
| Targets | `all` backends |
| Assertions | **OFF**, so call `verifyModule()` in builder programs to catch invalid IR |
| RTTI / exceptions | OFF (LLVM default), so builder programs must also use `-fno-rtti -fno-exceptions` |
| Linking | Static libraries (no `libLLVM.dylib`) |

Because this is a development snapshot, APIs are newer than many tutorials. For
example `Host.h` moved to `llvm/TargetParser/Host.h`, and pointers are opaque (`ptr`).

### Directory layout on disk

```
~/MY_LLVM/
├── llvm-project/     LLVM monorepo checkout (source) and build/ (the Ninja build tree)
├── llvm-basic-study/ this repo
└── LLVM_SETUP.md     detailed environment notes
```

The build is out-of-tree: source lives in `llvm-project/llvm`, generated files in
`llvm-project/build`. The `clang`, `llc`, `opt`, `lli` and `llvm-config` used here
are the self-built ones in `llvm-project/build/bin`, not Apple's.

### Shell setup (`~/.zshrc`)

```sh
export PATH="$HOME/MY_LLVM/llvm-project/build/bin:$PATH"   # self-built tools first
export SDKROOT=$(xcrun --show-sdk-path)                    # a self-built clang can't find the SDK otherwise
```

Without `SDKROOT`, even `#include <stdio.h>` fails with `'stdio.h' file not found`.

### Reproduce the build

```sh
brew install cmake ninja zstd
git clone https://github.com/llvm/llvm-project.git ~/MY_LLVM/llvm-project
cd ~/MY_LLVM/llvm-project && git checkout d26ea02060b1
cmake -S llvm -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_PROJECTS="clang" \
  -DLLVM_TARGETS_TO_BUILD="all"
cmake --build build
```

### Check your setup

```sh
which clang llc opt lli llvm-config     # all under llvm-project/build/bin
clang --version                         # clang version 22.0.0git
llvm-config --version --build-mode --assertion-mode --has-rtti
```

Expected: `22.0.0git`, `Release`, `OFF`, `NO`.

### Note on the committed `.ll` files

Each `.ll` file records the target triple of the machine and SDK it was generated on
(`arm64-apple-macosx26.0.0` in most, `macosx27.0.0` in the newest). They can differ
between lessons if the OS or Xcode was updated in between. This is harmless.

## Build and run a builder program

From inside a lesson folder:

```sh
clang++ hello_world_builder.cpp \
  `llvm-config --cxxflags --ldflags --system-libs --libs core support targetparser` \
  -L/opt/homebrew/lib -o hello_world_builder.out

./hello_world_builder.out        # prints / generates the IR
lli hello_world_builder.ll       # run the IR with the JIT interpreter
llc hello_world_builder.ll -o hello_world_builder.s   # lower the IR to assembly
```

Note: `llc` defaults to `-O2`, while `clang -S` defaults to `-O0`, so their assembly
can differ for the same program.

## Not tracked in git

Build output is ignored (see `.gitignore`): compiled `*.out` binaries and `.DS_Store`.
