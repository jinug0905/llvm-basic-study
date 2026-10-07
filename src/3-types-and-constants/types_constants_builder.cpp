// ============================== QUICK NOTES ==============================
// (Build line, ownership and SetInsertPoint: see the earlier files.)
//
// LLVM integer types have no signedness: `int` and `unsigned int` are both i32. Signedness lives
// in the OPERATION you pick, so the frontend has to choose it.
//
// Cast picker (C cast -> builder call -> IR):
//   widen signed int      CreateSExt              sext   copies the sign bit      int -> long
//   widen unsigned int    CreateZExt              zext   pads with zeros
//     (sext vs zext is decided by the SOURCE type's signedness, not the destination's)
//   narrow int            CreateTrunc             trunc  drops the high bits; signedness is irrelevant
//   signed int -> fp      CreateSIToFP            (unsigned source: CreateUIToFP)
//   fp -> signed int      CreateFPToSI            (unsigned target: CreateFPToUI)
//   float <-> double      CreateFPExt / CreateFPTrunc  (or CreateFPCast)
//   ptr <-> int           CreatePtrToInt / CreateIntToPtr
//   ptr -> ptr            nothing to do: every pointer is `ptr`. CreateBitCast is for same-size
//                         reinterpretation of non-pointers (e.g. i32 <-> float).
// The wrong sext/zext choice silently gives different numbers when the top bit is set.
//
// Pattern for `T2 y = (T2)x;`: load x (type T1), cast, store into y's alloca (type T2).
// Storing an i32 into an i64 slot without the cast is a type error that only verifyModule reports.
// Name every cast result ("i.sext"): the printed IR then reads like the C code instead of %0, %1, ...
//
// Check the result:  lli types_constants_builder.ll; echo $?   and compare with the compiled C binary.
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
    Module module("types_constants.c", context);
    IRBuilder<> builder(context);

    module.setSourceFileName("types_constants.c");
    module.setTargetTriple(Triple(sys::getDefaultTargetTriple()));

    // Declare `i32 @main()`
    FunctionType *mainTy = FunctionType::get(builder.getInt32Ty(), /*isVarArg=*/false);
    Function *mainFn = Function::Create(mainTy, Function::ExternalLinkage, "main", module);

    BasicBlock *entry = BasicBlock::Create(context, "entry", mainFn);
    builder.SetInsertPoint(entry);

    // int i = 65;
    auto* i = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "i");
    builder.CreateStore(builder.getInt32(65), i);

    // long l = i;  -> sext (int is signed, widening)
    auto *l = builder.CreateAlloca(builder.getInt64Ty(), nullptr, "l");
    auto *iVal1 = builder.CreateLoad(builder.getInt32Ty(), i);
    auto *iSext = builder.CreateSExt(iVal1, builder.getInt64Ty(), "i.sext");
    builder.CreateStore(iSext, l);

    // short s = (short)i; -> trunc (narrowing)
    auto *s = builder.CreateAlloca(builder.getInt16Ty(), nullptr, "s");
    auto *iVal2 = builder.CreateLoad(builder.getInt32Ty(), i);
    auto *iTrunc16 = builder.CreateTrunc(iVal2, builder.getInt16Ty(), "i.trunc16");
    builder.CreateStore(iTrunc16, s);

    // double d = i; -> sitofp (signed int -> float)
    auto *d = builder.CreateAlloca(builder.getDoubleTy(), nullptr, "d");
    auto *iVal3 = builder.CreateLoad(builder.getInt32Ty(), i);
    auto *iToDouble = builder.CreateSIToFP(iVal3, builder.getDoubleTy(), "i.sitofp");
    builder.CreateStore(iToDouble, d);

    // int back = (int)d; -> fptosi (float -> signed int)
    auto *back = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "back");
    auto *dVal = builder.CreateLoad(builder.getDoubleTy(), d);
    auto *dToInt = builder.CreateFPToSI(dVal, builder.getInt32Ty(), "d.fptosi");
    builder.CreateStore(dToInt, back);

    // char c = (char)i; -> trunc
    auto *c = builder.CreateAlloca(builder.getInt8Ty(), nullptr, "c");
    auto *iVal4 = builder.CreateLoad(builder.getInt32Ty(), i);
    auto *iTrunc8 = builder.CreateTrunc(iVal4, builder.getInt8Ty(), "i.trunc8");
    builder.CreateStore(iTrunc8, c);

    // return back;
    auto *retVal = builder.CreateLoad(builder.getInt32Ty(), back);
    builder.CreateRet(retVal);

    // Check the IR is well formed (assertions are off in this LLVM build, so do it explicitly).
    if (verifyModule(module, &errs())){
        errs() << "module verification failed\n";
        return 1;
    }

    // Print to the terminal and save to a file.
    module.print(outs(), nullptr);

    std::error_code ec;
    raw_fd_ostream out("types_constants_builder.ll", ec, sys::fs::OF_Text);
    if (ec){
        errs() << "cannot open output file: " << ec.message() << "\n";
        return 1;
    }
    
    module.print(out, nullptr);
    return 0;
}