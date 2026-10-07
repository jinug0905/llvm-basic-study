// ============================== QUICK NOTES ==============================
// (Build line, ownership and SetInsertPoint: see the earlier files.)
//
// Global vs local:
//   global: new GlobalVariable(module, type, isConstant, linkage, initializer, name)
//     - not an instruction: needs no insert point, and the Module owns it (no delete).
//     - its value is an ADDRESS (type ptr): you load/store through it.
//   local:  builder.CreateAlloca(type, nullptr, name)
//     - an instruction: needs an insert point. Returns the ADDRESS of the stack slot, never the value.
//     - takes no initializer: write the starting value with a separate CreateStore.
//   Reading a value: CreateLoad(type, ptr). A global can tell you its type
//   (getInitializer()->getType()); an alloca cannot, so you pass the type yourself every time.
//   CreateStore(value, ptr): pass the loaded VALUE. Storing the pointer itself builds bad IR that
//   only verifyModule notices.
//
// Two unrelated meanings of "constant":
//   llvm::Constant (ConstantInt, ConstantArray, ConstantStruct, ...) = a value known at compile time.
//   isConstant on a GlobalVariable = the memory is read-only after initialization.
//   GlobalVariable IS-A Constant (Constant <- GlobalValue <- GlobalObject <- GlobalVariable)
//   because its ADDRESS is fixed, even when its contents are mutable (global_a).
//   A mutable `int arr[4] = {...}` is built with the same ConstantArray::get; only isConstant differs.
//   A local array with an initializer is different: clang makes a private constant global
//   (@__const.main.arr) and memcpy's it into the alloca.
//
// Linkage (C -> LLVM):
//   no `static`              ExternalLinkage   other files can link to it     global_a
//   `static`, has a name     InternalLinkage   this file only, still named    const_arr, const_point, const_str
//   compiler-invented data   PrivateLinkage    not in the symbol table        the "hello" bytes (@.str)
//
// Building the constants:
//   array:   ArrayType::get(elemTy, N) + ConstantArray::get(arrTy, {values})
//   struct:  StructType::create(context, "struct.Point"), setBody({fieldTypes}),
//            ConstantStruct::get(structTy, {values})
//   string:  CreateGlobalString(text, name, 0, &module) makes the private [N x i8] global (adds \0);
//            const_str is a second, ptr-typed global that points at it.
//            Pass &module: by default it finds the module through the insert block and segfaults
//            if none is set (same root cause as CreateAlloca).
//
// Surprise: clang drops `static` globals that nothing uses, even at -O0, so clang's .ll has no
// const_arr/const_point/const_str. There is no clang IR to diff them against: rely on verifyModule.
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

int main()
{
    // 1. Context: owns types and constants. Module: holds the code. Builder: emits instructions.
    LLVMContext context;
    Module module("var_constant.c", context);
    IRBuilder<> builder(context);

    module.setSourceFileName("var_constant.c");
    module.setTargetTriple(Triple(sys::getDefaultTargetTriple()));

    // int global_a = 1;
    auto* global_a = new GlobalVariable(module, builder.getInt32Ty(),
                            false, GlobalValue::ExternalLinkage, builder.getInt32(1), "global_a");

    // static const int const_arr[4] = {1, 2, 3, 4};
    ArrayType* arrTy = ArrayType::get(builder.getInt32Ty(), 4);
    Constant* arrInit = ConstantArray::get(arrTy, { builder.getInt32(1), builder.getInt32(2),
                                                    builder.getInt32(3), builder.getInt32(4)});
    auto* const_arr = new GlobalVariable(module, arrTy, /*isConstant=*/true,
                                GlobalValue::InternalLinkage, arrInit, "const_arr");
    
    // struct Point { int x, y; };
    // static const struct Point const_point = {11, 12};
    StructType *pointTy = StructType::create(context, "struct.Point");
    pointTy->setBody({builder.getInt32Ty(), builder.getInt32Ty()});
    Constant *pointInit = ConstantStruct::get(pointTy, { builder.getInt32(11), builder.getInt32(12)});
    auto *const_point = new GlobalVariable(module, pointTy, /*isConstant=*/true,
                                GlobalValue::InternalLinkage, pointInit, "const_point");

    // static const char *const_str = "hello";
    GlobalVariable *strLiteral = builder.CreateGlobalString("hello", ".str", 0, &module);
    auto* const_str = new GlobalVariable(module, builder.getPtrTy(), /*isConstant=*/true,
                            GlobalValue::InternalLinkage, strLiteral, "const_str");

    // Declare `i32 @main()`
    FunctionType *mainTy = FunctionType::get(builder.getInt32Ty(), /*isVarArg=*/false);
    Function *mainFn = Function::Create(mainTy, Function::ExternalLinkage, "main", module);

    BasicBlock *entry = BasicBlock::Create(context, "entry", mainFn);
    builder.SetInsertPoint(entry);

    // local_b initialization and local_b = 2
    auto *local_b = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "local_b");
    builder.CreateStore(builder.getInt32(2), local_b);

    // local_b = global_a;
    auto global_a_rval = builder.CreateLoad(global_a->getInitializer()->getType(), global_a);
    builder.CreateStore(global_a_rval, local_b);

    // ret local_b
    auto *retVal = builder.CreateLoad(builder.getInt32Ty(), local_b);
    builder.CreateRet(retVal);

    // Check the IR is well formed (assertions are off in this LLVM build, so do it explicitly).
    if (verifyModule(module, &errs()))
    {
        errs() << "module verification failed\n";
        return 1;
    }

    // Print to the terminal and save to a file.
    module.print(outs(), nullptr);

    std::error_code ec;
    raw_fd_ostream out("var_constant_builder.ll", ec, sys::fs::OF_Text);
    if (ec)
    {
        errs() << "cannot open output file: " << ec.message() << "\n";
        return 1;
    }
    module.print(out, nullptr);
    return 0;
}