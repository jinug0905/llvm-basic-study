; ModuleID = 'pointers.c'
source_filename = "pointers.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx27.0.0"

%struct.Point = type { i32, i32 }

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define void @swap(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca ptr, align 8
  %4 = alloca ptr, align 8
  %5 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  store ptr %1, ptr %4, align 8
  %6 = load ptr, ptr %3, align 8
  %7 = load i32, ptr %6, align 4
  store i32 %7, ptr %5, align 4
  %8 = load ptr, ptr %4, align 8
  %9 = load i32, ptr %8, align 4
  %10 = load ptr, ptr %3, align 8
  store i32 %9, ptr %10, align 4
  %11 = load i32, ptr %5, align 4
  %12 = load ptr, ptr %4, align 8
  store i32 %11, ptr %12, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca [4 x i32], align 4
  %5 = alloca %struct.Point, align 4
  store i32 0, ptr %1, align 4
  store i32 1, ptr %2, align 4
  store i32 2, ptr %3, align 4
  call void @swap(ptr noundef %2, ptr noundef %3)
  %6 = getelementptr inbounds [4 x i32], ptr %4, i64 0, i64 0
  store i32 10, ptr %6, align 4
  %7 = getelementptr inbounds [4 x i32], ptr %4, i64 0, i64 1
  store i32 20, ptr %7, align 4
  %8 = getelementptr inbounds [4 x i32], ptr %4, i64 0, i64 2
  store i32 30, ptr %8, align 4
  %9 = getelementptr inbounds [4 x i32], ptr %4, i64 0, i64 3
  store i32 40, ptr %9, align 4
  %10 = getelementptr inbounds [4 x i32], ptr %4, i64 0, i64 0
  %11 = getelementptr inbounds [4 x i32], ptr %4, i64 0, i64 3
  call void @swap(ptr noundef %10, ptr noundef %11)
  %12 = getelementptr inbounds nuw %struct.Point, ptr %5, i32 0, i32 0
  store i32 5, ptr %12, align 4
  %13 = getelementptr inbounds nuw %struct.Point, ptr %5, i32 0, i32 1
  store i32 6, ptr %13, align 4
  %14 = getelementptr inbounds nuw %struct.Point, ptr %5, i32 0, i32 0
  %15 = getelementptr inbounds nuw %struct.Point, ptr %5, i32 0, i32 1
  call void @swap(ptr noundef %14, ptr noundef %15)
  %16 = load i32, ptr %2, align 4
  %17 = load i32, ptr %3, align 4
  %18 = sub nsw i32 %16, %17
  %19 = getelementptr inbounds [4 x i32], ptr %4, i64 0, i64 0
  %20 = load i32, ptr %19, align 4
  %21 = getelementptr inbounds [4 x i32], ptr %4, i64 0, i64 3
  %22 = load i32, ptr %21, align 4
  %23 = sub nsw i32 %20, %22
  %24 = add nsw i32 %18, %23
  %25 = getelementptr inbounds nuw %struct.Point, ptr %5, i32 0, i32 0
  %26 = load i32, ptr %25, align 4
  %27 = getelementptr inbounds nuw %struct.Point, ptr %5, i32 0, i32 1
  %28 = load i32, ptr %27, align 4
  %29 = sub nsw i32 %26, %28
  %30 = add nsw i32 %24, %29
  ret i32 %30
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
