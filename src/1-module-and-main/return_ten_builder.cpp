// ============================== QUICK NOTES ==============================
// (Build line, the three objects and ownership: see hello_world_builder.cpp.)
//
// From nothing to instructions:
//   FunctionType (just the shape)  ->  Function::Create (a declaration)
//   ->  BasicBlock::Create (gives it a body)  ->  builder.SetInsertPoint(block)  ->  builder.Create*()
//   - A basic block is straight-line code. Only its LAST instruction may branch or return (the
//     terminator), and every block needs exactly one. A block without one fails verifyModule.
//   - SetInsertPoint is load-bearing, not just "where to write". Calls such as CreateAlloca read the
//     insert block to find the module's DataLayout. With no insert point that block is null and the
//     program segfaults (exit 139).
//   - A bare Module is already valid, printable IR; it just has no functions yet.
//
// setDSOLocal(true) is optional here (a Linux/ELF idea):
//   - A "symbol" is a name the linker resolves to an address (main, printf, global_a).
//   - A symbol defined in ANOTHER shared library (printf in libc) has an address unknown until the
//     program loads, so code reaches it INDIRECTLY: read the address from a table (the GOT; calls
//     go through PLT stubs), then jump.
//   - A symbol defined in THIS binary has a fixed address at link time, so code can reach it
//     DIRECTLY: one instruction, no table.
//   - dso_local tells LLVM "this definition stays in this binary: address it directly".
//   - ELF = the Linux executable/library format (.so). macOS uses Mach-O, which has its own
//     mechanism, and clang does not emit dso_local for arm64-apple-macosx (see `define i32 @main()`
//     in clang's .ll). Keeping it is harmless and dropping it is fine: it is an addressing hint,
//     never a correctness requirement.
//   - It is unrelated to parameters: the parameter list is the function's TYPE, dso_local is about
//     its LINKAGE.
// =========================================================================

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Host.h"

#include <system_error>

using namespace llvm;

int main() {
  // 1. Context: owns types and constants. Module: holds the code. Builder: emits instructions.
  LLVMContext context;
  Module module("return_ten.c", context);
  IRBuilder<> builder(context);

  module.setSourceFileName("return_ten.c");
  module.setTargetTriple(Triple(sys::getDefaultTargetTriple()));

  // 2. Declare `i32 @main()`
  FunctionType *mainTy = FunctionType::get(builder.getInt32Ty(), /*isVarArg=*/false);
  Function *mainFn = Function::Create(mainTy, Function::ExternalLinkage, "main", module);
  mainFn->setDSOLocal(true);

  // 3. Entry block: everything below is appended here.
  BasicBlock *entry = BasicBlock::Create(context, "entry", mainFn);
  builder.SetInsertPoint(entry);

  // 4. return 10;
  builder.CreateRet(builder.getInt32(10));

  // 5. Check the IR is well formed (assertions are off in this LLVM build, so do it explicitly).
  if (verifyModule(module, &errs())) {
    errs() << "module verification failed\n";
    return 1;
  }

  // 6. Print to the terminal and save to a file.
  module.print(outs(), nullptr);

  std::error_code ec;
  raw_fd_ostream out("return_ten_builder.ll", ec, sys::fs::OF_Text);
  if (ec) {
    errs() << "cannot open output file: " << ec.message() << "\n";
    return 1;
  }
  module.print(out, nullptr);
  return 0;
}
