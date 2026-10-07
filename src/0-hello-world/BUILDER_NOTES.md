# LLVM IRBuilder — Refresher Notes

Notes on [hello_world_builder.cpp](hello_world_builder.cpp), written to pick
the concepts back up after a while away from LLVM (and after learning on an
older version at school).

## The big picture

Building IR is just: **build types → build a function skeleton → point the
builder at a block → append instructions one at a time**. That's it.
Everything below is one of those four steps.

## The three objects (context, module, builder)

```cpp
LLVMContext context;                       // line 24 — the shared universe: owns types, constants
Module module("hello_world.c", context);   // line 25 — one .c file's worth of code
IRBuilder<> builder(context);              // line 26 — the pen that writes instructions
```

## Step 1: Types before values

LLVM is statically typed at the IR level, so before you create anything you
describe its *shape* with a `Type`.

```cpp
FunctionType *printfTy = FunctionType::get(builder.getInt32Ty(), {builder.getPtrTy()}, /*isVarArg=*/true);
```

Read this as: "a function returning `i32`, taking one `ptr` argument, plus
varargs." This is the type of `printf`, not `printf` itself — no function
exists yet, just its shape. `builder.getInt32Ty()` and `getPtrTy()` are
convenience shortcuts on the builder for asking the Context for a type.

## Step 2: Declare vs. Define

This is the single most important idea in the file.

- **`getOrInsertFunction`** — used for `printf`. This makes a *declaration*:
  "this function exists somewhere, here's its signature," but no body.
  Matches `declare i32 @printf(ptr, ...)` in the IR.
- **`Function::Create`** — used for `main`. This also makes a *declaration*
  at first, but because you're about to give it a body, you'll turn it into
  a *definition*.

The difference in the IR: `declare` (no body, resolved at link time) vs.
`define` (body, present now).

## Step 3: A function is just a container for basic blocks

```cpp
BasicBlock *entry = BasicBlock::Create(context, "entry", mainFn);
builder.SetInsertPoint(entry);
```

A **basic block** is a straight-line run of instructions with no branches in
the middle — it only branches at the very end (or falls through). `main`
here has exactly one block, `entry`. `SetInsertPoint` moves the builder's pen
there. Every `builder.Create*` call after this line appends to `entry`, in
order, until you move the pen again.

For a function with `if`/loops, you'd call `BasicBlock::Create` multiple
times and set the insert point each time you switch which block you're
writing into. That's the whole model for control flow — see
`13_branch_if.cpp` and `15_loop_for.cpp` in the reference set.

## Step 4: Instructions are just `builder.Create*` calls, one line = one IR line

```cpp
Value *retSlot = builder.CreateAlloca(...);        // %retval = alloca i32
builder.CreateStore(builder.getInt32(0), retSlot); // store i32 0, ptr %retval
Value *str = builder.CreateGlobalString(...);      // @.str = ... (a global, not local!)
builder.CreateCall(printfTy, printfFn.getCallee(), {str}); // call i32 (ptr, ...) @printf(ptr @.str)
builder.CreateRet(builder.getInt32(0));            // ret i32 0
```

Two things worth re-noting:

- **Every `Create*` returns a `Value*`** — a handle to the result (an SSA
  register in IR, printed as `%something`). You use that handle as an
  argument to a later `Create*` call, exactly like `str` is fed into
  `CreateCall`.
- **`CreateGlobalString` is special** — despite being called mid-function, it
  doesn't emit a local instruction. It creates a *global* constant (`@.str`
  at module scope) and gives you back a pointer to it. That's why it doesn't
  show up as `%N` in `main`'s body.

## Step 5: Verify, because nothing stops you from writing garbage

```cpp
if (verifyModule(module, &errs())) { ... }
```

The builder API will happily let you build type-mismatched, malformed IR —
it's a construction API, not a checker. `verifyModule` is the sanity check
clang normally runs for you. Call it explicitly because this LLVM build has
assertions disabled (see [LLVM_SETUP.md](../../../LLVM_SETUP.md)), so LLVM
won't catch mistakes with a clear crash otherwise.

## What to drill next

The fastest way to re-solidify this: open the reference's numbered files in
order — `05_globals`, `06_locals`, `07_constants`, `10_arithmetic` — and for
each, before reading the code, predict what `Create*` calls you'd need, then
check yourself. The mental model is already there; the rest is just
vocabulary (which `Create*` maps to which IR instruction).
