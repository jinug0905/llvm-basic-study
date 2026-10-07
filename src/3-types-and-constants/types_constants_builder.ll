; ModuleID = 'types_constants.c'
source_filename = "types_constants.c"
target triple = "arm64-apple-darwin25.6.0"

define i32 @main() {
entry:
  %i = alloca i32, align 4
  store i32 65, ptr %i, align 4
  %l = alloca i64, align 8
  %0 = load i32, ptr %i, align 4
  %i.sext = sext i32 %0 to i64
  store i64 %i.sext, ptr %l, align 4
  %s = alloca i16, align 2
  %1 = load i32, ptr %i, align 4
  %i.trunc16 = trunc i32 %1 to i16
  store i16 %i.trunc16, ptr %s, align 2
  %d = alloca double, align 8
  %2 = load i32, ptr %i, align 4
  %i.sitofp = sitofp i32 %2 to double
  store double %i.sitofp, ptr %d, align 8
  %back = alloca i32, align 4
  %3 = load double, ptr %d, align 8
  %d.fptosi = fptosi double %3 to i32
  store i32 %d.fptosi, ptr %back, align 4
  %c = alloca i8, align 1
  %4 = load i32, ptr %i, align 4
  %i.trunc8 = trunc i32 %4 to i8
  store i8 %i.trunc8, ptr %c, align 1
  %5 = load i32, ptr %back, align 4
  ret i32 %5
}
