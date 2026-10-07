// ============================== QUICK NOTES ==============================
// (Build line, ownership, alloca/load/store: see the earlier files.)
//
// Control flow is a graph of basic blocks:
//   - A block is straight-line instructions ending in exactly ONE terminator: CreateBr,
//     CreateCondBr, CreateSwitch or CreateRet. No terminator, or an instruction written after
//     one, fails verifyModule. There is no fall-through in IR: execution goes only where a
//     terminator says.
//   - The edges ARE the control flow. A block is entered only because another block's terminator
//     names it. The `; preds = ...` comment on each label in the .ll lists its incoming edges.
//
// Three separate things (easy to mix up):
//   BasicBlock::Create(context, name, fn)  which function owns the block and where it is listed
//                                          (appended to the end of fn's block list at creation).
//   builder.SetInsertPoint(bb)             where the next instruction is written (the end of bb).
//                                          It moves a pen only and creates no edge. Everything
//                                          until the next SetInsertPoint lands in bb. Never write
//                                          into a block after its terminator.
//   CreateBr / CreateCondBr / CreateSwitch the actual execution edges.
//   Creating the blocks in a different order only changes the listing in the .ll, not behavior.
//
// if / else  (a > b):
//   create the then/else/end blocks up front, load a and b, CreateICmp(ICMP_SGT, ...) gives an
//   i1, CreateCondBr(cmp, thenBB, elseBB), fill each arm and end it with CreateBr(endBB), then
//   SetInsertPoint(endBB) and keep writing there. The i1 goes straight into CondBr: no zext.
//   clang's block names (clang -fno-discard-value-names keeps them): if.then, if.else, if.end.
//
// switch:
//   auto* sw = CreateSwitch(value, defaultBB, numCasesHint);  sw->addCase(ConstantInt*, caseBB);
//   - Keep the SwitchInst*: addCase is a method on it.
//   - `default` is the 2nd argument of CreateSwitch, never an addCase. numCases is only a hint.
//   - `break` is CreateBr(endBB). A missing break would branch to the next case's block instead.
//   - switch is a terminator: it ends the block you are in.
//   - clang's names: sw.bb, sw.bb1, sw.default, sw.epilog.
//
// Values vs memory across blocks:
//   A value (the result of a Create* call) can only be used where its defining instruction
//   dominates the use, i.e. every path to the use goes through it. Using the `then` block's load
//   in if.end fails with "Instruction does not dominate all uses!". That is why variables live in
//   allocas at -O0: `result` is written in then/else and read later through memory (mem2reg turns
//   this into phi nodes later). Reusing aV, defined in entry, inside then/else would be legal,
//   because entry dominates both.
//
// Silent trap: storing 0 instead of 2 into `level` still verifies, but takes the default branch
// and returns 77. Expected exit code is 87 (if gives 7, switch case 2 adds 80):
//   lli branches_builder.ll; echo $?
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
    Module module("branches.c", context);
    IRBuilder<> builder(context);

    module.setSourceFileName("branches.c");
    module.setTargetTriple(Triple(sys::getDefaultTargetTriple()));

    // Declare `i32 @main()`
    FunctionType *mainTy = FunctionType::get(builder.getInt32Ty(), /*isVarArg=*/false);
    Function *mainFn = Function::Create(mainTy, Function::ExternalLinkage, "main", module);

    BasicBlock *entry = BasicBlock::Create(context, "entry", mainFn);
    builder.SetInsertPoint(entry);

    // int a = 3, b = 7;
    auto* a = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "a");
    auto* b = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "b");
    builder.CreateStore(builder.getInt32(3), a);
    builder.CreateStore(builder.getInt32(7), b);

    // int result = 0;
    auto* result = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "result");
    builder.CreateStore(builder.getInt32(0), result);

    // if (a > b) { result = a; } else { result = b; }
    BasicBlock* thenBB = BasicBlock::Create(context, "if.then", mainFn);
    BasicBlock* elseBB = BasicBlock::Create(context, "if.else", mainFn);
    BasicBlock* endBB = BasicBlock::Create(context, "if.end", mainFn);

    auto* aV = builder.CreateLoad(builder.getInt32Ty(), a, "a.val");
    auto* bV = builder.CreateLoad(builder.getInt32Ty(), b, "b.val");
    auto* cmp = builder.CreateICmp(CmpInst::ICMP_SGT, aV, bV, "cmp");
    builder.CreateCondBr(cmp, thenBB, elseBB);       // ends the entry block

    // then: result = a;
    builder.SetInsertPoint(thenBB);
    auto* aThen = builder.CreateLoad(builder.getInt32Ty(), a, "a.then");
    builder.CreateStore(aThen, result);
    builder.CreateBr(endBB);                         // ends if.then

    // else: result = b;
    builder.SetInsertPoint(elseBB);
    auto* bElse = builder.CreateLoad(builder.getInt32Ty(), b, "b.else");
    builder.CreateStore(bElse, result);
    builder.CreateBr(endBB);                         // ends if.else

    // everything after the if/else continues here
    builder.SetInsertPoint(endBB);

    // int level = 2;
    auto* level = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "level");
    builder.CreateStore(builder.getInt32(2), level);
    auto* levelVal = builder.CreateLoad(builder.getInt32Ty(), level, "level.val");

    // switch (level) { case 1: ... case 2: ... default: ... }
    BasicBlock* sw1 = BasicBlock::Create(context, "sw.1", mainFn);
    BasicBlock* sw2 = BasicBlock::Create(context, "sw.2", mainFn);
    BasicBlock* swDefault = BasicBlock::Create(context, "sw.default", mainFn);
    BasicBlock* swEnd = BasicBlock::Create(context, "sw.end", mainFn);

    auto* sw = builder.CreateSwitch(levelVal, swDefault, 2);   // ends if.end
    sw->addCase(builder.getInt32(1), sw1);
    sw->addCase(builder.getInt32(2), sw2);

    // case 1: result = result + 90; break;
    builder.SetInsertPoint(sw1);
    auto* r1 = builder.CreateLoad(builder.getInt32Ty(), result, "r1");
    builder.CreateStore(builder.CreateNSWAdd(r1, builder.getInt32(90), "r1.add"), result);
    builder.CreateBr(swEnd);                                   // break

    // case 2: result = result + 80; break;
    builder.SetInsertPoint(sw2);
    auto* r2 = builder.CreateLoad(builder.getInt32Ty(), result, "r2");
    builder.CreateStore(builder.CreateNSWAdd(r2, builder.getInt32(80), "r2.add"), result);
    builder.CreateBr(swEnd);

    // default: result = result + 70; break;
    builder.SetInsertPoint(swDefault);
    auto* rd = builder.CreateLoad(builder.getInt32Ty(), result, "rd");
    builder.CreateStore(builder.CreateNSWAdd(rd, builder.getInt32(70), "rd.add"), result);
    builder.CreateBr(swEnd);

    // return result;
    builder.SetInsertPoint(swEnd);
    builder.CreateRet(builder.CreateLoad(builder.getInt32Ty(), result, "ret.val"));

    // Check the IR is well formed (assertions are off in this LLVM build, so do it explicitly).
    if (verifyModule(module, &errs())){
        errs() << "module verification failed\n";
        return 1;
    }

    // Print to the terminal and save to a file.
    module.print(outs(), nullptr);

    std::error_code ec;
    raw_fd_ostream out("branches_builder.ll", ec, sys::fs::OF_Text);
    if (ec){
        errs() << "cannot open output file: " << ec.message() << "\n";
        return 1;
    }
    
    module.print(out, nullptr);
    return 0;
}