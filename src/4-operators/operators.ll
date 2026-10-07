; ModuleID = 'operators.c'
source_filename = "operators.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx26.0.0"

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  %11 = alloca i32, align 4
  %12 = alloca i32, align 4
  %13 = alloca i32, align 4
  %14 = alloca i32, align 4
  %15 = alloca i32, align 4
  %16 = alloca i32, align 4
  %17 = alloca i32, align 4
  %18 = alloca i32, align 4
  %19 = alloca i32, align 4
  %20 = alloca double, align 8
  %21 = alloca double, align 8
  %22 = alloca i32, align 4
  %23 = alloca i32, align 4
  %24 = alloca i32, align 4
  %25 = alloca i32, align 4
  %26 = alloca double, align 8
  %27 = alloca double, align 8
  %28 = alloca double, align 8
  %29 = alloca double, align 8
  %30 = alloca double, align 8
  %31 = alloca i32, align 4
  %32 = alloca i32, align 4
  %33 = alloca i32, align 4
  %34 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  store i32 17, ptr %2, align 4
  store i32 5, ptr %3, align 4
  %35 = load i32, ptr %2, align 4
  %36 = load i32, ptr %3, align 4
  %37 = add nsw i32 %35, %36
  store i32 %37, ptr %4, align 4
  %38 = load i32, ptr %2, align 4
  %39 = load i32, ptr %3, align 4
  %40 = sub nsw i32 %38, %39
  store i32 %40, ptr %5, align 4
  %41 = load i32, ptr %2, align 4
  %42 = load i32, ptr %3, align 4
  %43 = mul nsw i32 %41, %42
  store i32 %43, ptr %6, align 4
  %44 = load i32, ptr %2, align 4
  %45 = load i32, ptr %3, align 4
  %46 = sdiv i32 %44, %45
  store i32 %46, ptr %7, align 4
  %47 = load i32, ptr %2, align 4
  %48 = load i32, ptr %3, align 4
  %49 = srem i32 %47, %48
  store i32 %49, ptr %8, align 4
  store i32 17, ptr %9, align 4
  store i32 5, ptr %10, align 4
  %50 = load i32, ptr %9, align 4
  %51 = load i32, ptr %10, align 4
  %52 = udiv i32 %50, %51
  store i32 %52, ptr %11, align 4
  %53 = load i32, ptr %2, align 4
  %54 = load i32, ptr %3, align 4
  %55 = and i32 %53, %54
  store i32 %55, ptr %12, align 4
  %56 = load i32, ptr %2, align 4
  %57 = load i32, ptr %3, align 4
  %58 = or i32 %56, %57
  store i32 %58, ptr %13, align 4
  %59 = load i32, ptr %2, align 4
  %60 = load i32, ptr %3, align 4
  %61 = xor i32 %59, %60
  store i32 %61, ptr %14, align 4
  %62 = load i32, ptr %2, align 4
  %63 = shl i32 %62, 2
  store i32 %63, ptr %15, align 4
  %64 = load i32, ptr %2, align 4
  %65 = ashr i32 %64, 2
  store i32 %65, ptr %16, align 4
  %66 = load i32, ptr %9, align 4
  %67 = lshr i32 %66, 2
  store i32 %67, ptr %17, align 4
  %68 = load i32, ptr %2, align 4
  %69 = load i32, ptr %3, align 4
  %70 = icmp slt i32 %68, %69
  %71 = zext i1 %70 to i32
  store i32 %71, ptr %18, align 4
  %72 = load i32, ptr %2, align 4
  %73 = load i32, ptr %3, align 4
  %74 = icmp eq i32 %72, %73
  %75 = zext i1 %74 to i32
  store i32 %75, ptr %19, align 4
  store double 3.140000e+00, ptr %20, align 8
  store double 2.000000e+00, ptr %21, align 8
  %76 = load double, ptr %20, align 8
  %77 = load double, ptr %21, align 8
  %78 = fcmp olt double %76, %77
  %79 = zext i1 %78 to i32
  store i32 %79, ptr %22, align 4
  %80 = load i32, ptr %2, align 4
  %81 = sub nsw i32 0, %80
  store i32 %81, ptr %23, align 4
  %82 = load i32, ptr %2, align 4
  %83 = xor i32 %82, -1
  store i32 %83, ptr %24, align 4
  %84 = load i32, ptr %2, align 4
  %85 = icmp ne i32 %84, 0
  %86 = xor i1 %85, true
  %87 = zext i1 %86 to i32
  store i32 %87, ptr %25, align 4
  %88 = load double, ptr %20, align 8
  %89 = fneg double %88
  store double %89, ptr %26, align 8
  %90 = load double, ptr %20, align 8
  %91 = load double, ptr %21, align 8
  %92 = fadd double %90, %91
  store double %92, ptr %27, align 8
  %93 = load double, ptr %20, align 8
  %94 = load double, ptr %21, align 8
  %95 = fsub double %93, %94
  store double %95, ptr %28, align 8
  %96 = load double, ptr %20, align 8
  %97 = load double, ptr %21, align 8
  %98 = fmul double %96, %97
  store double %98, ptr %29, align 8
  %99 = load double, ptr %20, align 8
  %100 = load double, ptr %21, align 8
  %101 = fdiv double %99, %100
  store double %101, ptr %30, align 8
  %102 = load i32, ptr %9, align 4
  %103 = add i32 %102, 1
  store i32 %103, ptr %31, align 4
  %104 = load i32, ptr %2, align 4
  %105 = add nsw i32 %104, 1
  store i32 %105, ptr %32, align 4
  %106 = load i32, ptr %2, align 4
  %107 = sub nsw i32 %106, 1
  store i32 %107, ptr %33, align 4
  %108 = load i32, ptr %2, align 4
  %109 = mul nsw i32 %108, 2
  store i32 %109, ptr %34, align 4
  %110 = load i32, ptr %4, align 4
  ret i32 %110
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
