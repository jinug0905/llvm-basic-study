; ModuleID = 'branches.c'
source_filename = "branches.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx26.0.0"

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  store i32 3, ptr %2, align 4
  store i32 7, ptr %3, align 4
  store i32 0, ptr %4, align 4
  %6 = load i32, ptr %2, align 4
  %7 = load i32, ptr %3, align 4
  %8 = icmp sgt i32 %6, %7
  br i1 %8, label %9, label %11

9:                                                ; preds = %0
  %10 = load i32, ptr %2, align 4
  store i32 %10, ptr %4, align 4
  br label %13

11:                                               ; preds = %0
  %12 = load i32, ptr %3, align 4
  store i32 %12, ptr %4, align 4
  br label %13

13:                                               ; preds = %11, %9
  store i32 2, ptr %5, align 4
  %14 = load i32, ptr %5, align 4
  switch i32 %14, label %21 [
    i32 1, label %15
    i32 2, label %18
  ]

15:                                               ; preds = %13
  %16 = load i32, ptr %4, align 4
  %17 = add nsw i32 %16, 90
  store i32 %17, ptr %4, align 4
  br label %24

18:                                               ; preds = %13
  %19 = load i32, ptr %4, align 4
  %20 = add nsw i32 %19, 80
  store i32 %20, ptr %4, align 4
  br label %24

21:                                               ; preds = %13
  %22 = load i32, ptr %4, align 4
  %23 = add nsw i32 %22, 70
  store i32 %23, ptr %4, align 4
  br label %24

24:                                               ; preds = %21, %18, %15
  %25 = load i32, ptr %4, align 4
  ret i32 %25
}

attributes #0 = { noinline nounwind optnone ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 27, i32 0]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 1}
!5 = !{!"clang version 22.0.0git (https://github.com/llvm/llvm-project.git d26ea02060b1c9db751d188b2edb0059a9eb273d)"}
