// ============================== QUICK NOTES ==============================
// (Build line, ownership, blocks and terminators: see the earlier files.)
//
// Structure of any multi-function module:
//   1. setup     context, module, builder: shared by every function
//   2. declare   all the FunctionTypes and Function::Create calls (getOrInsertFunction for
//                externals like printf), BEFORE any body
//   3. define    one section per function: BasicBlock::Create(context, "entry", thatFunction),
//                SetInsertPoint, instructions, a terminator
//   4. verify, print, save
//
// Declare vs define (a function IS a declaration until it has a block):
//   - A function with no basic blocks prints as `declare`; give it a block and it becomes `define`.
//     printf stays a declaration: its body lives in libc and is resolved when linking.
//   - Every defined function needs at least one block, add and show included. Block names only have
//     to be unique inside one function: each function can have its own `entry`, and a duplicate
//     inside one function is renamed entry1, entry2, ...
//   - A call needs its callee to exist already, which is why everything is declared up front.
//     The listing order in the .ll follows the order of the Function::Create calls.
//
// FunctionType::get(returnType, {paramTypes}, isVarArg): the return type comes FIRST.
//   - void show(int) is get(getVoidTy(), {getInt32Ty()}, false). Swapping the first two gives a
//     function with a void parameter: "Function arguments must have first-class types!".
//   - printf is get(i32, {ptr}, true): {ptr} is the fixed format parameter, true allows the rest.
//
// Parameters:
//   - fn->getArg(i) is the incoming value; setName("x") only makes the IR readable.
//   - clang -O0 pattern: alloca "x.addr", store the argument into it, load it wherever it is used.
//   - An argument belongs to its own function. Using one function's argument inside another fails
//     with "Referring to an argument in another function!".
//
// Calls:
//   - CreateCall(fn, {args}, name) returns the result value; the name is optional. A void call has
//     no value to name (show(r)).
//   - Argument count and types must match exactly, LLVM converts nothing:
//       missing argument  ->  "Incorrect number of arguments passed to called function!"
//       i64 where i32 is expected  ->  "Call parameter type does not match function signature!"
//     (cast with sext/trunc first).
//   - Vararg (printf): declare with isVarArg=true and pass the full type to the call:
//     CreateCall(printfTy, printfFn.getCallee(), {fmt, value}). The format string comes FIRST;
//     passing the int in its place is a parameter type mismatch.
//
// Returns: CreateRet(value) for int functions, CreateRetVoid() for void functions.
//
// The classic mistake: forgetting SetInsertPoint when starting the next function. The pen is still at
// the end of the previous function, so the new body lands inside it: "Terminator found in the middle
// of a basic block!" plus "Referring to an argument in another function!".
//
// Strings: CreateGlobalString(text, name, 0, &module) makes the private @.str and returns a ptr that
// goes straight into the call. With opaque pointers no GEP is needed. Pass &module: otherwise it
// finds the module through the insert block.
//
// =========================================================================

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/IR/Constants.h"

#include <system_error>

using namespace llvm;

int main(){
    // 1. Context: owns types and constants. Module: holds the code. Builder: emits instructions.
    LLVMContext context;
    Module module("functions.c", context);
    IRBuilder<> builder(context);

    module.setSourceFileName("functions.c");
    module.setTargetTriple(Triple(sys::getDefaultTargetTriple()));

    // 2. Declare every function first (signature only, no body yet).
    //    A call can only be built once its callee exists, so all of them are declared up front.
    FunctionType* mainTy = FunctionType::get(builder.getInt32Ty(), /*isVarArg=*/false);
    FunctionType* addTy = FunctionType::get(builder.getInt32Ty(), {builder.getInt32Ty(), builder.getInt32Ty()}, false);
    FunctionType* showTy = FunctionType::get(builder.getVoidTy(), {builder.getInt32Ty()}, false);
    FunctionType* printfTy = FunctionType::get(builder.getInt32Ty(), {builder.getPtrTy()}, /*isVarArg=*/true);

    Function* mainFn = Function::Create(mainTy, Function::ExternalLinkage, "main", module);
    Function* addFn = Function::Create(addTy, llvm::Function::ExternalLinkage, "add", module);
    Function* showFn = Function::Create(showTy, llvm::Function::ExternalLinkage, "show", module);
    FunctionCallee printfFn = module.getOrInsertFunction("printf", printfTy);   // stays a declaration: defined in libc

    // 3. Define each function, in the same order as the C file. Every definition follows the same recipe:
    //      BasicBlock::Create(context, "entry", thatFunction)  ->  builder.SetInsertPoint(entry)
    //      ->  emit instructions  ->  end the block with a terminator (ret).
    //    The builder is one shared pen: move it into the new function before writing anything.

    // int add(int x, int y) { return x + y; }
    addFn->getArg(0)->setName("x");
    addFn->getArg(1)->setName("y");
    BasicBlock *addEntry = BasicBlock::Create(context, "entry", addFn);
    builder.SetInsertPoint(addEntry);

    // parameters live in memory like every other variable: alloca, then store the incoming argument
    auto* xAddr = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "x.addr");
    auto* yAddr = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "y.addr");
    builder.CreateStore(addFn->getArg(0), xAddr);
    builder.CreateStore(addFn->getArg(1), yAddr);

    // return x + y;
    auto* xV = builder.CreateLoad(builder.getInt32Ty(), xAddr, "x.val");
    auto* yV = builder.CreateLoad(builder.getInt32Ty(), yAddr, "y.val");
    builder.CreateRet(builder.CreateNSWAdd(xV, yV, "add"));

    // -=-=-=-=-=-=-=-=-=-=-

    // void show(int v) { printf("value: %d\n", v); }
    showFn->getArg(0)->setName("v");
    BasicBlock *showEntry = BasicBlock::Create(context, "entry", showFn);
    builder.SetInsertPoint(showEntry);

    auto* vAddr = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "v.addr");
    builder.CreateStore(showFn->getArg(0), vAddr);

    // printf("value: %d\n", v);   the format string first, then the value
    auto* fmt = builder.CreateGlobalString("value: %d\n", ".str", 0, &module);
    auto* vVal = builder.CreateLoad(builder.getInt32Ty(), vAddr, "v.val");
    builder.CreateCall(printfTy, printfFn.getCallee(), {fmt, vVal}, "call");
    builder.CreateRetVoid();                                       // void function: no value to return

    // -=-=-=-=-=-=-=-=-=-=-

    // int main() { int r = add(3, 4); show(r); return r; }
    BasicBlock* entry = BasicBlock::Create(context, "entry", mainFn);
    builder.SetInsertPoint(entry);

    // int r = add(3, 4);
    auto* r = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "r");
    auto* callAddFn = builder.CreateCall(addFn, {builder.getInt32(3), builder.getInt32(4)}, "calladdFn");
    builder.CreateStore(callAddFn, r);

    // show(r);
    auto* rShow = builder.CreateLoad(builder.getInt32Ty(), r, "r.show");
    builder.CreateCall(showFn, {rShow});      // void result, so no name

    // return r;
    auto* rRet = builder.CreateLoad(builder.getInt32Ty(), r, "r.ret");
    builder.CreateRet(rRet);

    // 4. Check the IR is well formed (assertions are off in this LLVM build, so do it explicitly).
    if (verifyModule(module, &errs())){
        errs() << "module verification failed\n";
        return 1;
    }

    // 5. Print to the terminal and save to a file.
    module.print(outs(), nullptr);

    std::error_code ec;
    raw_fd_ostream out("functions_builder.ll", ec, sys::fs::OF_Text);
    if (ec){
        errs() << "cannot open output file: " << ec.message() << "\n";
        return 1;
    }

    module.print(out, nullptr);
    return 0;
}
