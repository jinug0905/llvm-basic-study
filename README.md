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

## Prerequisites

- LLVM installed with `llvm-config`, `clang`, `lli` and `llc` on your `PATH`
  (the build line below assumes Homebrew on Apple Silicon: `/opt/homebrew`)

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
