// ============================== QUICK NOTES ==============================
// (Build line, ownership, blocks, branches, loops, functions: see the earlier files.)
//
// SSA in one line: every value is defined exactly once and never changes afterwards.
//   - The earlier chapters kept variables in allocas and changed them with load/store (clang -O0
//     style). An SSA value cannot change, so a variable that changes (i and s in a loop) or that
//     depends on the path taken (a ternary) needs a phi instead of memory.
//   - Function arguments are already SSA values: use them directly, with no a.addr alloca, no
//     store and no load. clang spills arguments to allocas at -O0 only because that is the simple
//     thing to generate, and mem2reg removes them again.
//   - opt -passes=mem2reg turns the alloca style into this style. Run it on clang's -O0 IR (compile
//     with -Xclang -disable-O0-optnone, otherwise optnone makes the pass skip the function) and it
//     gives the same four phis as this file: cond, land, i, s.
//
// phi = "pick a value by which predecessor block control came from":
//   %cond = phi i32 [ %a, %cond.true ], [ %b, %cond.false ]
//   - builder.CreatePHI(type, expectedIncomingCount, name), then phi->addIncoming(value, block)
//     once per incoming edge.
//   - The block is the PREDECESSOR control flows in from (the block whose branch targets the phi's
//     block), not the block where the value was computed. s.next is computed in for.body, but it
//     arrives from for.inc.
//   - The incoming value must dominate the end of that predecessor block. That is why a phi may use
//     a value defined later in the loop (the back edge).
//   - Every predecessor needs exactly one entry, and phis must be the first instructions in the block.
//
// Where phis come from in the source:
//   ?:    cond.true / cond.false / cond.end, with the phi in cond.end.
//   &&    land.rhs / land.end, with phi i1 [false, entry], [rhs, land.rhs]. The right side is only
//         evaluated when the left side was true. CreateAnd would evaluate both sides: it is bitwise,
//         not short-circuit.
//   ||    the mirror image: lor.rhs / lor.end, phi i1 [true, entry], [rhs, lor.rhs], and the entry
//         branch goes to the end block when the left side is TRUE.
//   loop  one phi per variable that changes inside the loop, at the top of the loop header.
//   A comparison or && gives an i1; extend it (zext) to i32 to return it as an int.
//
// Building a loop phi (the back-edge problem): the back-edge value (i.next, s.next) does not exist
// yet when the phi is created.
//   1. CreatePHI in the loop header and add only the entry edge ([1, entry]).
//   2. Build the body and the increment, which creates i.next and s.next.
//   3. Come back and close it: i->addIncoming(iNext, forInc). The phi pointer stays valid.
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
    Module module("phi.c", context);
    IRBuilder<> builder(context);

    module.setSourceFileName("phi.c");
    module.setTargetTriple(Triple(sys::getDefaultTargetTriple()));

    // 2. Declare every function first (signature only, no body yet).
    FunctionType* maxTy = FunctionType::get(builder.getInt32Ty(), {builder.getInt32Ty(), builder.getInt32Ty()}, false);
    FunctionType* bothpTy = FunctionType::get(builder.getInt32Ty(), {builder.getInt32Ty(), builder.getInt32Ty()}, false);
    FunctionType* sumtoTy = FunctionType::get(builder.getInt32Ty(), {builder.getInt32Ty()}, false);
    FunctionType* mainTy = FunctionType::get(builder.getInt32Ty(), false);

    Function* maxFn = Function::Create(maxTy, Function::ExternalLinkage, "max", module);
    Function* bothpFn = Function::Create(bothpTy, Function::ExternalLinkage, "both_positive", module);
    Function* sumtoFn = Function::Create(sumtoTy, Function::ExternalLinkage, "sum_to", module);
    Function* mainFn = Function::Create(mainTy, Function::ExternalLinkage, "main", module);

    // 3. Define each function, in the same order as the C file.
    //    These functions are written in pure SSA: the arguments are already values, so there are no
    //    allocas, loads or stores. A value that depends on which path was taken is a phi instead.

    // int max(int a, int b) { return a > b ? a : b; }
    maxFn->getArg(0)->setName("a");
    maxFn->getArg(1)->setName("b");
    BasicBlock* maxEntry = BasicBlock::Create(context, "entry", maxFn);
    BasicBlock* condTrue = BasicBlock::Create(context, "cond.true", maxFn);
    BasicBlock* condFalse = BasicBlock::Create(context, "cond.false", maxFn);
    BasicBlock* condEnd = BasicBlock::Create(context, "cond.end", maxFn);

    // a > b ?   branch to one of the two arms
    builder.SetInsertPoint(maxEntry);
    auto* aGtB = builder.CreateICmp(CmpInst::ICMP_SGT, maxFn->getArg(0), maxFn->getArg(1), "cmp");
    builder.CreateCondBr(aGtB, condTrue, condFalse);

    // each arm just continues to the join block (the value it contributes is the phi's job)
    builder.SetInsertPoint(condTrue);
    builder.CreateBr(condEnd);
    builder.SetInsertPoint(condFalse);
    builder.CreateBr(condEnd);

    // join: the result is `a` if we came from cond.true, `b` if we came from cond.false
    builder.SetInsertPoint(condEnd);
    auto* cond = builder.CreatePHI(builder.getInt32Ty(), 2, "cond");
    cond->addIncoming(maxFn->getArg(0), condTrue);
    cond->addIncoming(maxFn->getArg(1), condFalse);
    builder.CreateRet(cond);

    // -=-=-=-=-=-=-=-=-=-=-

    // int both_positive(int a, int b) { return a > 0 && b > 0; }
    // && short-circuits: `b > 0` must only be evaluated when `a > 0` was true.
    bothpFn->getArg(0)->setName("a");
    bothpFn->getArg(1)->setName("b");
    BasicBlock* bothpEntry = BasicBlock::Create(context, "entry", bothpFn);
    BasicBlock* landRhs = BasicBlock::Create(context, "land.rhs", bothpFn);
    BasicBlock* landEnd = BasicBlock::Create(context, "land.end", bothpFn);

    // a > 0 ?   false: skip the right side, true: go evaluate it
    builder.SetInsertPoint(bothpEntry);
    auto* aPos = builder.CreateICmp(CmpInst::ICMP_SGT, bothpFn->getArg(0), builder.getInt32(0), "a.pos");
    builder.CreateCondBr(aPos, landRhs, landEnd);

    // right side: b > 0
    builder.SetInsertPoint(landRhs);
    auto* bPos = builder.CreateICmp(CmpInst::ICMP_SGT, bothpFn->getArg(1), builder.getInt32(0), "b.pos");
    builder.CreateBr(landEnd);

    // join: false if we skipped straight from entry, otherwise the value of `b > 0`.
    // It is an i1 (a comparison result), so extend it to the i32 the function returns.
    builder.SetInsertPoint(landEnd);
    auto* land = builder.CreatePHI(builder.getInt1Ty(), 2, "land");
    land->addIncoming(builder.getFalse(), bothpEntry);
    land->addIncoming(bPos, landRhs);
    builder.CreateRet(builder.CreateZExt(land, builder.getInt32Ty(), "land.ext"));

    // -=-=-=-=-=-=-=-=-=-=-

    // int sum_to(int n) { int s = 0; for (int i = 1; i <= n; i++) { s = s + i; } return s; }
    // A loop in pure SSA: i and s change every iteration, so each one is a phi at the top of the loop header.
    sumtoFn->getArg(0)->setName("n");
    BasicBlock* sumEntry = BasicBlock::Create(context, "entry", sumtoFn);
    BasicBlock* forCond = BasicBlock::Create(context, "for.cond", sumtoFn);
    BasicBlock* forBody = BasicBlock::Create(context, "for.body", sumtoFn);
    BasicBlock* forInc = BasicBlock::Create(context, "for.inc", sumtoFn);
    BasicBlock* forEnd = BasicBlock::Create(context, "for.end", sumtoFn);

    builder.SetInsertPoint(sumEntry);
    builder.CreateBr(forCond);

    // for.cond: the phis come FIRST in the block, before any other instruction.
    // Only the entry edge is known now; the back edge is added once its value exists.
    builder.SetInsertPoint(forCond);
    auto* i = builder.CreatePHI(builder.getInt32Ty(), 2, "i");
    auto* s = builder.CreatePHI(builder.getInt32Ty(), 2, "s");
    i->addIncoming(builder.getInt32(1), sumEntry);                 // i = 1 on the way in
    s->addIncoming(builder.getInt32(0), sumEntry);                 // s = 0 on the way in
    auto* iLeN = builder.CreateICmp(CmpInst::ICMP_SLE, i, sumtoFn->getArg(0), "cmp");
    builder.CreateCondBr(iLeN, forBody, forEnd);

    // for.body: s = s + i;
    builder.SetInsertPoint(forBody);
    auto* sNext = builder.CreateNSWAdd(s, i, "s.next");
    builder.CreateBr(forInc);

    // for.inc: i++
    builder.SetInsertPoint(forInc);
    auto* iNext = builder.CreateNSWAdd(i, builder.getInt32(1), "i.next");
    builder.CreateBr(forCond);                                     // back edge

    // now that i.next and s.next exist, close the phis with the back edge.
    // The block is the PREDECESSOR the value arrives from (for.inc), not where it was computed
    // (for.body computed s.next, but control reaches for.cond from for.inc).
    i->addIncoming(iNext, forInc);
    s->addIncoming(sNext, forInc);

    // for.end: the phi from for.cond holds the final s
    builder.SetInsertPoint(forEnd);
    builder.CreateRet(s);

    // -=-=-=-=-=-=-=-=-=-=-

    // int main() { return max(3, 9) + both_positive(1, 2) + sum_to(10); }
    BasicBlock* entry = BasicBlock::Create(context, "entry", mainFn);
    builder.SetInsertPoint(entry);

    auto* callMax = builder.CreateCall(maxFn, {builder.getInt32(3), builder.getInt32(9)}, "call.max");
    auto* callBothp = builder.CreateCall(bothpFn, {builder.getInt32(1), builder.getInt32(2)}, "call.bothp");
    auto* partial = builder.CreateNSWAdd(callMax, callBothp, "partial");
    auto* callSumto = builder.CreateCall(sumtoFn, {builder.getInt32(10)}, "call.sumto");
    builder.CreateRet(builder.CreateNSWAdd(partial, callSumto, "ret.val"));

    // 4. Check the IR is well formed (assertions are off in this LLVM build, so do it explicitly).
    if (verifyModule(module, &errs())){
        errs() << "module verification failed\n";
        return 1;
    }

    // 5. Print to the terminal and save to a file.
    module.print(outs(), nullptr);

    std::error_code ec;
    raw_fd_ostream out("phi_builder.ll", ec, sys::fs::OF_Text);
    if (ec){
        errs() << "cannot open output file: " << ec.message() << "\n";
        return 1;
    }

    module.print(out, nullptr);
    return 0;
}
