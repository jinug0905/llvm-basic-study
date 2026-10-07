// ============================== QUICK NOTES ==============================
//
// Build and run:
//   clang++ hello_world_builder.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core support targetparser` -L/opt/homebrew/lib -o hello_world_builder.out
//   - Backticks (not quotes): the shell runs llvm-config and pastes its output in.
//   - --cxxflags: LLVM include paths plus -fno-rtti -fno-exceptions (must match how LLVM was built).
//   - --libs core support targetparser: LLVM component libraries. An "undefined symbol" at link
//     time usually means a component is missing from this list.
//   - --system-libs asks for -lzstd; it lives in /opt/homebrew/lib, hence the extra -L.
//   Run the generated IR:  lli hello_world_builder.ll   (exit code = main's return value & 0xFF)
//
// The three objects:
//   LLVMContext  the shared universe: owns types and constants (there is only one i32, ever).
//   Module       one translation unit: holds functions, globals, declarations.
//   IRBuilder    a cursor that appends instructions at its insert point. It owns no IR itself.
//
// Why there is no delete anywhere (ownership):
//   - A Module owns everything inside it: Functions, GlobalVariables, BasicBlocks, Instructions.
//     Function::Create(..., module) and new GlobalVariable(module, ...) hand the object to the
//     Module; builder.Create*() hands each instruction to its BasicBlock. Everything is freed when
//     the Module is destroyed, so never delete these yourself.
//   - Types and constants belong to the Context, not the Module.
//   - Declare the LLVMContext BEFORE the Module. Locals die in reverse order, and the Module has
//     to be destroyed while the Context is still alive.
//
// Declare vs define:
//   declare = signature only, no body (printf: the body lives in libc).
//   define  = has a body (main).
//   Function::Create and getOrInsertFunction both give you the declaration; it becomes a
//   definition once you add a BasicBlock to it.
//
// Other things used here:
//   - Opaque pointers: every pointer type is just `ptr`. No i8*, no pointer-to-pointer casts.
//   - CreateGlobalString makes the private global @.str holding "text\0" and returns it.
//   - verifyModule: this LLVM build has assertions OFF, so malformed IR does not crash, it just
//     prints. Always verify explicitly.
//   - getDefaultTargetTriple() asks the host machine (arm64-apple-darwin25.x). clang prints
//     arm64-apple-macosx26.0.0 for the same machine. Harmless difference.
//   - Hand-built IR is leaner than clang -O0: no dead retval alloca, no attribute groups, no noundef.
// =========================================================================

#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Host.h"

#include <memory>
#include <string>
#include <system_error>

using namespace llvm;

int main() {
  // 1. Context: owns types and constants. Module: holds the code. Builder: emits instructions.
  LLVMContext context;
  Module module("hello_world.c", context);
  IRBuilder<> builder(context);

  module.setSourceFileName("hello_world.c");
  module.setTargetTriple(Triple(sys::getDefaultTargetTriple()));

  // 2. Declare `i32 @printf(ptr, ...)`
  //    Every pointer is just `ptr` (opaque pointers), no more i8*.
  FunctionType *printfTy = FunctionType::get(builder.getInt32Ty(), {builder.getPtrTy()}, /*isVarArg=*/true);
  FunctionCallee printfFn = module.getOrInsertFunction("printf", printfTy);

  // 3. Define `i32 @main()`
  FunctionType *mainTy = FunctionType::get(builder.getInt32Ty(), /*isVarArg=*/false);
  Function *mainFn = Function::Create(mainTy, Function::ExternalLinkage, "main", module);

  // 4. Entry block: everything below is appended here.
  BasicBlock *entry = BasicBlock::Create(context, "entry", mainFn);
  builder.SetInsertPoint(entry);

  // 5. Match clang -O0: reserve a slot for the return value and store 0 into it.
  Value *retSlot = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "retval");
  builder.CreateStore(builder.getInt32(0), retSlot);

  // 6. Global string constant "Hello World !!!\n" (adds the trailing \0 for us),
  //    then call printf with it.
  Value *str = builder.CreateGlobalString("Hello World !!!\n", ".str");
  builder.CreateCall(printfTy, printfFn.getCallee(), {str});

  // 7. return 0
  builder.CreateRet(builder.getInt32(0));

  // 8. Check the IR is well formed (assertions are off in this LLVM build, so do it explicitly).
  if (verifyModule(module, &errs())) {
    errs() << "module verification failed\n";
    return 1;
  }

  // 9. Print to the terminal and save to a file.
  module.print(outs(), nullptr);

  std::error_code ec;
  raw_fd_ostream out("hello_world_builder.ll", ec, sys::fs::OF_Text);
  if (ec) {
    errs() << "cannot open output file: " << ec.message() << "\n";
    return 1;
  }
  module.print(out, nullptr);
  return 0;
}
