// ============================== QUICK NOTES ==============================
// (Build line, ownership, alloca/load/store: see the earlier files. Blocks, terminators,
// SetInsertPoint and CreateCondBr: see branches_builder.cpp.)
//
// A loop is just blocks with one BACKWARD edge:
//   - The loop header (for.cond / while.cond) has two predecessors: the code before the loop and
//     the back edge from the end of the loop. In the .ll: `for.cond: ; preds = %for.inc, %entry`.
//   - The header runs once more than the body (11 times for 10 iterations): the last check is the
//     one that fails and leaves the loop.
//   - The loop variable lives in an alloca: written in the increment, read in the condition and
//     the body. The back edge needs nothing special (mem2reg turns this into a phi later).
//
// for (init; cond; inc) { body }  ->  4 blocks:
//   entry:     init, CreateBr(for.cond)
//   for.cond:  load i, CreateICmp(SLE) gives an i1, CreateCondBr(cmp, for.body, for.end)
//   for.body:  body, CreateBr(for.inc)
//   for.inc:   inc, CreateBr(for.cond)                      <- the back edge
//   for.end:   the code after the loop
//
// while (cond) { body }  ->  3 blocks (while.cond, while.body, while.end):
//   the same shape without an increment block. The body ends with the back edge
//   CreateBr(while.cond), and `j = j + 1` is just a statement in the body.
//   Why `for` has the extra block: the increment is its own piece of syntax, and a `continue` in a
//   for loop must jump to it. In a while loop `continue` jumps to the condition. clang emits
//   for.inc even when the loop has no `continue`.
//
// Allocas go in the entry block (all of them: i, and j/prod even though they are declared later):
//   - An alloca takes NEW stack space every time it executes. In a loop body that is every
//     iteration: 10M iterations overflowed the stack (exit 139). In the entry block it runs once.
//   - mem2reg only promotes allocas that are in the entry block.
//   Where the alloca goes and where the initial store goes are separate: j and prod are allocated
//   up top, but `j = 1; prod = 1;` is stored where the C code has it (for.end, before the while).
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
    Module module("loops.c", context);
    IRBuilder<> builder(context);

    module.setSourceFileName("loops.c");
    module.setTargetTriple(Triple(sys::getDefaultTargetTriple()));

    // Declare `i32 @main()`
    FunctionType *mainTy = FunctionType::get(builder.getInt32Ty(), /*isVarArg=*/false);
    Function *mainFn = Function::Create(mainTy, Function::ExternalLinkage, "main", module);

    BasicBlock *entry = BasicBlock::Create(context, "entry", mainFn);
    builder.SetInsertPoint(entry);

    // all of main's allocas go in the entry block, even ones declared later in the C code
    auto* sum = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "sum");
    auto* i = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "i");
    auto* j = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "j");
    auto* prod = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "prod");

    // int sum = 0;
    builder.CreateStore(builder.getInt32(0), sum);

    // for (int i = 1; i <= 10; i++) { sum = sum + i; }
    BasicBlock* forCond = BasicBlock::Create(context, "for.cond", mainFn);
    BasicBlock* forBody = BasicBlock::Create(context, "for.body", mainFn);
    BasicBlock* forInc = BasicBlock::Create(context, "for.inc", mainFn);
    BasicBlock* forEnd = BasicBlock::Create(context, "for.end", mainFn);

    // init: i = 1, then jump into the loop
    builder.CreateStore(builder.getInt32(1), i);
    builder.CreateBr(forCond);                                   // entry -> for.cond

    // for.cond: i <= 10 ?
    builder.SetInsertPoint(forCond);
    auto* iCond = builder.CreateLoad(builder.getInt32Ty(), i, "i.cond");
    auto* icmp = builder.CreateICmp(CmpInst::ICMP_SLE, iCond, builder.getInt32(10), "i.cmp");
    builder.CreateCondBr(icmp, forBody, forEnd);                  // true: body, false: leave

    // for.body: sum = sum + i;
    builder.SetInsertPoint(forBody);
    auto* sumV = builder.CreateLoad(builder.getInt32Ty(), sum, "sum.val");
    auto* iBody = builder.CreateLoad(builder.getInt32Ty(), i, "i.body");
    builder.CreateStore(builder.CreateNSWAdd(sumV, iBody, "sum.add"), sum);
    builder.CreateBr(forInc);                                    // body -> for.inc

    // for.inc: i++
    builder.SetInsertPoint(forInc);
    auto* iInc = builder.CreateLoad(builder.getInt32Ty(), i, "i.inc");
    builder.CreateStore(builder.CreateNSWAdd(iInc, builder.getInt32(1), "i.next"), i);
    builder.CreateBr(forCond);                                   // back edge: for.inc -> for.cond

    // for.end: the code after the loop is written here
    builder.SetInsertPoint(forEnd);

    // while (j <= 5) { prod = prod * j; j = j + 1; }
    BasicBlock* whileCond = BasicBlock::Create(context, "while.cond", mainFn);
    BasicBlock* whileBody = BasicBlock::Create(context, "while.body", mainFn);
    BasicBlock* whileEnd = BasicBlock::Create(context, "while.end", mainFn);

    // init: j = 1; prod = 1; then jump into the loop (we are still in for.end)
    builder.CreateStore(builder.getInt32(1), j);
    builder.CreateStore(builder.getInt32(1), prod);
    builder.CreateBr(whileCond);                                   // for.end -> while.cond

    // while.cond: j <= 5 ?
    builder.SetInsertPoint(whileCond);
    auto* jCond = builder.CreateLoad(builder.getInt32Ty(), j, "j.cond");
    auto* jcmp = builder.CreateICmp(CmpInst::ICMP_SLE, jCond, builder.getInt32(5), "j.cmp");
    builder.CreateCondBr(jcmp, whileBody, whileEnd);               // true: body, false: leave

    // while.body: prod = prod * j; j = j + 1;
    builder.SetInsertPoint(whileBody);
    auto* prodV = builder.CreateLoad(builder.getInt32Ty(), prod, "prod.val");
    auto* jBody = builder.CreateLoad(builder.getInt32Ty(), j, "j.body");
    builder.CreateStore(builder.CreateNSWMul(prodV, jBody, "prod.mul"), prod);
    auto* jInc = builder.CreateLoad(builder.getInt32Ty(), j, "j.inc");
    builder.CreateStore(builder.CreateNSWAdd(jInc, builder.getInt32(1), "j.next"), j);
    builder.CreateBr(whileCond);                                   // back edge: while.body -> while.cond

    // return sum + prod;
    builder.SetInsertPoint(whileEnd);
    auto* sumRet = builder.CreateLoad(builder.getInt32Ty(), sum, "sum.ret");
    auto* prodRet = builder.CreateLoad(builder.getInt32Ty(), prod, "prod.ret");
    builder.CreateRet(builder.CreateNSWAdd(sumRet, prodRet, "ret.val"));

    // Check the IR is well formed (assertions are off in this LLVM build, so do it explicitly).
    if (verifyModule(module, &errs())){
        errs() << "module verification failed\n";
        return 1;
    }

    // Print to the terminal and save to a file.
    module.print(outs(), nullptr);

    std::error_code ec;
    raw_fd_ostream out("loops_builder.ll", ec, sys::fs::OF_Text);
    if (ec){
        errs() << "cannot open output file: " << ec.message() << "\n";
        return 1;
    }
    
    module.print(out, nullptr);
    return 0;
}