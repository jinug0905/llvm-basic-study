// Builds the IR of pointers.c with the LLVM C++ API.
//
//   struct Point { int x; int y; };
//   void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }
//   main: swap a plain variable, an array element and a struct field; returns 32
//
// ============================== QUICK NOTES ==============================
// (Build line, ownership, blocks, functions and calls: see the earlier files.)
//
// Opaque pointers: the central change from older LLVM material.
//   - Every pointer has the same type, `ptr` (getPtrTy()). It does NOT remember what it points to.
//   - So the pointee type is passed explicitly every time: CreateAlloca(type), CreateLoad(type, ptr),
//     CreateInBoundsGEP(type, ptr, ...), CreateStructGEP(type, ptr, ...). Keep arrTy / pointTy around.
//   - getNonOpaquePointerElementType(), used all over the old reference, no longer exists.
//   - Consequence: the verifier cannot check what a pointer points to, so pointer mistakes verify
//     and then misbehave (see "Silent mistakes" below).
//
// Address vs value (the whole chapter in one line): an alloca, a global or a GEP result IS an
// address. You load to get a value and store to write one.
//   - `&a` of a local is just the alloca itself, already an address: swap(&a, &b) is
//     CreateCall(swapFn, {a, b}). Passing the loaded ints is a parameter type mismatch.
//   - An `int *a` parameter gets a slot of type `alloca ptr` (a.addr) that holds the pointer.
//
// Dereferencing takes two loads (this is clang's swap):
//   int t = *a;  ->  aPtr = CreateLoad(ptr, a.addr);  aVal = CreateLoad(i32, aPtr);  store aVal into t
//   *a = *b;     ->  store bVal THROUGH a freshly loaded pointer (a.ptr2), never into the a.addr slot.
//
// GEP (getelementptr) computes an address and reads no memory: you still load/store through it.
//   array   CreateInBoundsGEP(arrTy, arr, {0, i})
//             - the first index steps over the whole array (always 0 for one array), the second
//               picks the element
//             - i64 by convention (pointer width), but any integer width is accepted and it can be
//               a runtime value
//   struct  CreateStructGEP(pointTy, p, field)  emits {i32 0, i32 field}
//             - the FIELD index must be a constant i32: {i32 0, i64 1} is rejected with
//               "Invalid indices for GEP pointer type!"
//             - the leading 0 may be any integer type
//   clang prints `inbounds nuw` on struct field GEPs, and the builder's output matches.
//   An address can be computed once and reused (arr0, arr3, px, py); clang recomputes it at each use.
//
// Local arrays and structs are allocas, not constant globals:
//   `int arr[4]` and `struct Point p`, which swap writes into, are `alloca [4 x i32]` and
//   `alloca %struct.Point` in main's entry block, filled with stores through GEPs. A `constant`
//   global (the constants chapter) is read-only: writing into it crashes at run time.
//
// Silent mistakes (all of these verify; only running it, or a diff against clang, shows them):
//   - storing the value into the pointer's slot (a.addr) instead of through the loaded pointer:
//     wrong answer (exit 0 instead of 32)
//   - one index {i} in an array GEP instead of {0, i}: it steps over whole arrays and crashes
//   - a pointer parameter's slot made `alloca i32` instead of `alloca ptr`
//
// Differences from clang's IR that are fine: one extra alloca/store in clang (the retval slot),
// more GEPs in clang (see above), and `%p` is align 8 here vs align 4 in clang (IRBuilder uses the
// type's preferred alignment).
//
// Check the result:  lli pointers_builder.ll; echo $?   -> 32
//   (a - b) = 1 for the variable swap, (arr[0] - arr[3]) = 30 for the array swap,
//   (p.x - p.y) = 1 for the struct swap
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
    Module module("pointers.c", context);
    IRBuilder<> builder(context);

    module.setSourceFileName("pointers.c");
    module.setTargetTriple(Triple(sys::getDefaultTargetTriple()));

    // 2. Declare the types and both functions first (signature only, no body yet).
    //    Pointers carry no pointee type any more (every pointer is just `ptr`), so the array and
    //    struct types are kept here and passed explicitly to every alloca / GEP / load below.
    ArrayType* arrTy = ArrayType::get(builder.getInt32Ty(), 4);

    StructType* pointTy = StructType::create(context, "struct.Point");
    pointTy->setBody({builder.getInt32Ty(), builder.getInt32Ty()});

    FunctionType* mainTy = FunctionType::get(builder.getInt32Ty(), /*isVarArg=*/false);
    FunctionType* swapTy = FunctionType::get(builder.getVoidTy(), {builder.getPtrTy(), builder.getPtrTy()}, false);

    Function* mainFn = Function::Create(mainTy, Function::ExternalLinkage, "main", module);
    Function* swapFn = Function::Create(swapTy, llvm::Function::ExternalLinkage, "swap", module);

    // 3. Define each function, in the same order as the C file.

    // void swap(int *a, int *b) { int t = *a; *a = *b; *b = t;}
    swapFn->getArg(0)->setName("a");
    swapFn->getArg(1)->setName("b");
    BasicBlock *swapEntry = BasicBlock::Create(context, "swap.entry", swapFn);
    builder.SetInsertPoint(swapEntry);

    // The parameters are POINTERS, so their slots are `alloca ptr`. `t` is a plain int.
    auto* aSlot = builder.CreateAlloca(builder.getPtrTy(), nullptr, "a.addr");
    auto* bSlot = builder.CreateAlloca(builder.getPtrTy(), nullptr, "b.addr");
    auto* tSlot = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "t");

    builder.CreateStore(swapFn->getArg(0), aSlot);
    builder.CreateStore(swapFn->getArg(1), bSlot);

    // int t = *a;      two loads: first the pointer out of its slot, then the int through the pointer
    auto* aPtr = builder.CreateLoad(builder.getPtrTy(), aSlot, "a.ptr");
    auto* aVal = builder.CreateLoad(builder.getInt32Ty(), aPtr, "a.val");
    builder.CreateStore(aVal, tSlot);

    // *a = *b;         store the int THROUGH the loaded pointer, not into the pointer's slot
    auto* bPtr = builder.CreateLoad(builder.getPtrTy(), bSlot, "b.ptr");
    auto* bVal = builder.CreateLoad(builder.getInt32Ty(), bPtr, "b.val");
    auto* aPtr2 = builder.CreateLoad(builder.getPtrTy(), aSlot, "a.ptr2");
    builder.CreateStore(bVal, aPtr2);

    // *b = t;
    auto* tVal = builder.CreateLoad(builder.getInt32Ty(), tSlot, "t.val");
    auto* bPtr2 = builder.CreateLoad(builder.getPtrTy(), bSlot, "b.ptr2");
    builder.CreateStore(tVal, bPtr2);
    builder.CreateRetVoid();

    // -=-=-=-=-=-=-=-=-=-=-

    // int main()
    BasicBlock* entry = BasicBlock::Create(context, "entry", mainFn);
    builder.SetInsertPoint(entry);

    // int a = 1, b = 2;   int arr[4];   struct Point p;
    // All of main's allocas go in the entry block. arr and p are local, mutable variables
    // (swap writes into them), so they are allocas and NOT constant globals.
    auto* a = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "a");
    auto* b = builder.CreateAlloca(builder.getInt32Ty(), nullptr, "b");
    auto* arr = builder.CreateAlloca(arrTy, nullptr, "arr");
    auto* p = builder.CreateAlloca(pointTy, nullptr, "p");

    builder.CreateStore(builder.getInt32(1), a);
    builder.CreateStore(builder.getInt32(2), b);

    // swap(&a, &b);    &a is just the alloca: it already is an address
    builder.CreateCall(swapFn, {a, b});                      // void call: no name

    // arr[0] = 10; arr[1] = 20; arr[2] = 30; arr[3] = 40;
    // An element address is a GEP with two indices {0, i}: the first steps over the whole array
    // (always 0 for one array), the second picks the element. A GEP only computes an address:
    // you still store through it.
    for (int i = 0; i < 4; i++) {
        auto* elem = builder.CreateInBoundsGEP(arrTy, arr, {builder.getInt64(0), builder.getInt64(i)}, "arrayidx");
        builder.CreateStore(builder.getInt32(10 * (i + 1)), elem);
    }

    // swap(&arr[0], &arr[3]);
    auto* arr0 = builder.CreateInBoundsGEP(arrTy, arr, {builder.getInt64(0), builder.getInt64(0)}, "arr0");
    auto* arr3 = builder.CreateInBoundsGEP(arrTy, arr, {builder.getInt64(0), builder.getInt64(3)}, "arr3");
    builder.CreateCall(swapFn, {arr0, arr3});

    // p.x = 5; p.y = 6;     a field address is a struct GEP: the struct type and the field index
    auto* px = builder.CreateStructGEP(pointTy, p, 0, "p.x");
    auto* py = builder.CreateStructGEP(pointTy, p, 1, "p.y");
    builder.CreateStore(builder.getInt32(5), px);
    builder.CreateStore(builder.getInt32(6), py);

    // swap(&p.x, &p.y);
    builder.CreateCall(swapFn, {px, py});

    // return (a - b) + (arr[0] - arr[3]) + (p.x - p.y);
    // The values are loaded AFTER the swaps; the addresses computed above can be reused.
    auto* aV = builder.CreateLoad(builder.getInt32Ty(), a, "a.v");
    auto* bV = builder.CreateLoad(builder.getInt32Ty(), b, "b.v");
    auto* diffAB = builder.CreateNSWSub(aV, bV, "diff.ab");

    auto* arr0V = builder.CreateLoad(builder.getInt32Ty(), arr0, "arr0.v");
    auto* arr3V = builder.CreateLoad(builder.getInt32Ty(), arr3, "arr3.v");
    auto* diffArr = builder.CreateNSWSub(arr0V, arr3V, "diff.arr");

    auto* pxV = builder.CreateLoad(builder.getInt32Ty(), px, "px.v");
    auto* pyV = builder.CreateLoad(builder.getInt32Ty(), py, "py.v");
    auto* diffP = builder.CreateNSWSub(pxV, pyV, "diff.p");

    builder.CreateRet(builder.CreateNSWAdd(builder.CreateNSWAdd(diffAB, diffArr), diffP, "ret.val"));

    // 4. Check the IR is well formed (assertions are off in this LLVM build, so do it explicitly).
    if (verifyModule(module, &errs())){
        errs() << "module verification failed\n";
        return 1;
    }

    // 5. Print to the terminal and save to a file.
    module.print(outs(), nullptr);

    std::error_code ec;
    raw_fd_ostream out("pointers_builder.ll", ec, sys::fs::OF_Text);
    if (ec){
        errs() << "cannot open output file: " << ec.message() << "\n";
        return 1;
    }

    module.print(out, nullptr);
    return 0;
}
