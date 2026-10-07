// ============================== QUICK NOTES ==============================
// (Build line, ownership and SetInsertPoint: see the earlier files.)
//
// Signedness lives in the opcode (see types_constants_builder.cpp). Pick it from the C type:
//   / and %     CreateSDiv, CreateSRem  (int)        vs  CreateUDiv, CreateURem  (unsigned)
//   >>          CreateAShr (int: copies the sign bit) vs CreateLShr (unsigned: zero fill)
//   < <= > >=   ICMP_SLT/SLE/SGT/SGE     (int)        vs  ICMP_ULT/ULE/UGT/UGE    (unsigned)
//   == !=       ICMP_EQ / ICMP_NE (same for both)
//   + - * << & | ^   one opcode for both signednesses
// Check with a = -17: sdiv -17,5 = -3 (exit code 253) but udiv gives 47; ashr by 28 gives 255 but
// lshr gives 15. (A shift of 2 gives 251 for both: exit codes keep only the low 8 bits.)
//
// Comparisons return i1, not i32. A C `int` result needs CreateZExt(i1 -> i32) before the store.
// When the result feeds a branch, use the i1 directly (CreateCondBr), no zext.
//
// Float compares: CreateFCmp, never CreateICmp. FCMP predicates are ORDERED or UNORDERED:
//   FCMP_OLT = "less than, and neither operand is NaN"   FCMP_ULT = "less than, or either is NaN"
//   C `<` is ordered, so clang emits `fcmp olt`.
// Float arithmetic: CreateFAdd / FSub / FMul / FDiv / FRem, and CreateFNeg.
//
// Unary operators have no opcode of their own:
//   -a  = sub nsw 0, a    CreateNSWNeg          ~a = xor a, -1    CreateNot
//   !a  = icmp ne a, 0, then CreateNot on the i1, then zext to i32
//   -d (double) = fneg    CreateFNeg
//
// Overflow flags: nsw (no signed wrap) and nuw (no unsigned wrap) are a PROMISE to the optimizer
// that the operation never wraps. Breaking the promise gives poison (undefined behavior).
//   - C signed int overflow is undefined: CreateNSWAdd / NSWSub / NSWMul / NSWNeg.
//   - C unsigned arithmetic wraps by definition: plain CreateAdd. clang puts no flag on shl either.
//   - What it buys (opt -passes=instcombine): `x + 1 > x` folds to `true` with nsw; without the
//     flag it stays `x != INT_MAX`.
//   - Only use nsw if the source language says overflow is undefined. Wrapping languages use plain ops.
//
// Pitfalls:
//   - CreateLoad(Type, Ptr, Name): argument order matters. CreateLoad(ty, nullptr, somePtr) COMPILES:
//     somePtr converts to bool and selects the (Type, Ptr, bool isVolatile) overload, i.e. a
//     volatile load from null.
//   - Wrong-variable and wrong-opcode slips (stored into `a` instead of `ua`, URem for a division)
//     still verify, and the exit code only reflects `sum`. Compare against clang instead:
//     clang -S -emit-llvm -O0 operators.c, then check the same opcodes and flags appear.
//   - Loading a, b, ... once and reusing the value is fine while nothing stores in between.
//     clang -O0 reloads at every use. Both are valid IR.
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

int main()
{
    // 1. Context: owns types and constants. Module: holds the code. Builder: emits instructions.
    LLVMContext context;
    Module module("operators.c", context);
    IRBuilder<> builder(context);

    module.setSourceFileName("operators.c");
    module.setTargetTriple(Triple(sys::getDefaultTargetTriple()));

    // Declare `i32 @main()`
    FunctionType *mainTy = FunctionType::get(builder.getInt32Ty(), /*isVarArg=*/false);
    Function *mainFn = Function::Create(mainTy, Function::ExternalLinkage, "main", module);

    BasicBlock *entry = BasicBlock::Create(context, "entry", mainFn);
    builder.SetInsertPoint(entry);

    // int a = 17;
    auto* a = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "a");
    builder.CreateStore(builder.getInt32(17), a);

    // int b = 5;
    auto* b = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "b");
    builder.CreateStore(builder.getInt32(5), b);

    //  unsigned int ua = 17;
    auto* ua = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "ua");
    builder.CreateStore(builder.getInt32(17), ua);

    //  unsigned int ub = 5;
    auto* ub = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "ub");
    builder.CreateStore(builder.getInt32(5), ub);

    // double x = 3.14;
    auto* x = builder.CreateAlloca(builder.getDoubleTy(), nullptr, "x");
    builder.CreateStore(ConstantFP::get(builder.getDoubleTy(), 3.14), x);

    // double y = 2.0;
    auto* y = builder.CreateAlloca(builder.getDoubleTy(), nullptr, "y");
    builder.CreateStore(ConstantFP::get(builder.getDoubleTy(), 2.0), y);

    auto *aV = builder.CreateLoad(builder.getInt32Ty(), a, "a.val");
    auto *bV = builder.CreateLoad(builder.getInt32Ty(), b, "b.val");
    auto *uaV = builder.CreateLoad(builder.getInt32Ty(), ua, "ua.val");
    auto *ubV = builder.CreateLoad(builder.getInt32Ty(), ub, "ub.val");
    auto *xV = builder.CreateLoad(builder.getDoubleTy(), x, "x.val");
    auto *yV = builder.CreateLoad(builder.getDoubleTy(), y, "y.val");

    // int sum = a + b;
    auto *sum = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "sum");
    builder.CreateStore(builder.CreateAdd(aV, bV, "add"), sum);

    // int diff = a - b;
    auto *diff = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "diff");
    builder.CreateStore(builder.CreateSub(aV, bV, "sub"), diff);

    // int prod = a * b;
    auto *prod = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "prod");
    builder.CreateStore(builder.CreateMul(aV, bV, "mul"), prod);

    // int quot = a / b;
    auto *quot = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "quot");
    builder.CreateStore(builder.CreateSDiv(aV, bV, "sdiv"), quot);

    // int rem = a % b;
    auto *rem = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "rem");
    builder.CreateStore(builder.CreateSRem(aV, bV, "srem"), rem);

    // unsigned int uquot = ua / ub;
    auto *uquot = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "uquot");
    builder.CreateStore(builder.CreateUDiv(uaV, ubV, "udiv"), uquot);

    // int band = a & b;
    auto *band = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "band");
    builder.CreateStore(builder.CreateAnd(aV, bV, "and"), band);

    // int bor = a | b;
    auto *bor = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "bor");
    builder.CreateStore(builder.CreateOr(aV, bV, "or"), bor);

    // int bxor = a ^ b;
    auto *bxor = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "bxor");
    builder.CreateStore(builder.CreateXor(aV, bV, "xor"), bxor);

    // int lsh  = a << 2;
    auto *lsh = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "lsh");
    builder.CreateStore(builder.CreateShl(aV, builder.getInt32(2), "shl"), lsh);

    // int rsh  = a >> 2;
    auto *rsh = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "rsh");
    builder.CreateStore(builder.CreateAShr(aV, builder.getInt32(2), "ashr"), rsh);

    // unsigned int ursh = ua >> 2;
    auto *ursh = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "ursh");
    builder.CreateStore(builder.CreateLShr(uaV, builder.getInt32(2), "lshr"), ursh);

    // int lt = a < b;
    // Comparisons yield i1 in LLVM, but a C `int` is i32 so we need to zext before storing.
    auto *lt = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "lt");
    auto *ltCmp = builder.CreateICmp(CmpInst::ICMP_SLT, aV, bV, "cmp.slt");
    builder.CreateStore(builder.CreateZExt(ltCmp, builder.getInt32Ty(), "lt.conv"), lt);

    // int eq = a == b;
    auto *eq = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "eq");
    auto *eqCmp = builder.CreateICmp(CmpInst::ICMP_EQ, aV, bV, "cmp.eq");
    builder.CreateStore(builder.CreateZExt(eqCmp, builder.getInt32Ty(), "eq.conv"), eq);

    // int flt_lt = x < y;
    auto *fltLt = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "flt_lt");
    auto *fltCmp = builder.CreateFCmp(CmpInst::FCMP_OLT, xV, yV, "cmp.olt");
    builder.CreateStore(builder.CreateZExt(fltCmp, builder.getInt32Ty(), "flt_lt.conv"), fltLt);

    // int neg = -a;     clang: sub nsw i32 0, %a
    auto *neg = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "neg");
    builder.CreateStore(builder.CreateNSWNeg(aV, "neg.val"), neg);

    // int bnot = ~a;    clang: xor i32 %a, -1
    auto *bnot = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "bnot");
    builder.CreateStore(builder.CreateNot(aV, "not.val"), bnot);

    // int lnot = !a;    clang: icmp ne a, 0 -> xor i1 true -> zext to i32
    auto *lnot = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "lnot");
    auto *tobool = builder.CreateICmpNE(aV, builder.getInt32(0), "tobool"); // a != 0   -> i1
    auto *lnotI1 = builder.CreateNot(tobool, "lnot.i1");                    // flip it  -> i1
    builder.CreateStore(builder.CreateZExt(lnotI1, builder.getInt32Ty(), "lnot.ext"), lnot); // i1 -> i32

    // double dneg = -x;
    auto *dneg = builder.CreateAlloca(builder.getDoubleTy(), nullptr, "dneg");
    builder.CreateStore(builder.CreateFNeg(xV, "fneg"), dneg);

    // double dsum = x + y;
    auto *dsum = builder.CreateAlloca(builder.getDoubleTy(), nullptr, "dsum");
    builder.CreateStore(builder.CreateFAdd(xV, yV, "fadd"), dsum);

    // double ddiff = x - y;
    auto *ddiff = builder.CreateAlloca(builder.getDoubleTy(), nullptr, "ddiff");
    builder.CreateStore(builder.CreateFSub(xV, yV, "fsub"), ddiff);

    // double dprod = x * y;
    auto *dprod = builder.CreateAlloca(builder.getDoubleTy(), nullptr, "dprod");
    builder.CreateStore(builder.CreateFMul(xV, yV, "fmul"), dprod);

    // double dquot = x / y;
    auto *dquot = builder.CreateAlloca(builder.getDoubleTy(), nullptr, "dquot");
    builder.CreateStore(builder.CreateFDiv(xV, yV, "fdiv"), dquot);

    // unsigned int uadd = ua + 1;   plain add: unsigned overflow wraps, so no flag
    auto *uadd = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "uadd");
    builder.CreateStore(builder.CreateAdd(uaV, builder.getInt32(1), "uadd.val"), uadd);

    // int sadd = a + 1;   add nsw: signed overflow is undefined in C
    auto *sadd = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "sadd");
    builder.CreateStore(builder.CreateNSWAdd(aV, builder.getInt32(1), "sadd.val"), sadd);

    // int ssub = a - 1;   sub nsw
    auto *ssub = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "ssub");
    builder.CreateStore(builder.CreateNSWSub(aV, builder.getInt32(1), "ssub.val"), ssub);

    // int smul = a * 2;   mul nsw
    auto *smul = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "smul");
    builder.CreateStore(builder.CreateNSWMul(aV, builder.getInt32(2), "smul.val"), smul);

    builder.CreateRet(builder.CreateLoad(builder.getInt32Ty(), sum, "ret.val"));

    // Check the IR is well formed (assertions are off in this LLVM build, so do it explicitly).
    if (verifyModule(module, &errs())){
        errs() << "module verification failed\n";
        return 1;
    }

    // Print to the terminal and save to a file.
    module.print(outs(), nullptr);

    std::error_code ec;
    raw_fd_ostream out("operators_builder.ll", ec, sys::fs::OF_Text);
    if (ec){
        errs() << "cannot open output file: " << ec.message() << "\n";
        return 1;
    }
    
    module.print(out, nullptr);
    return 0;
}
