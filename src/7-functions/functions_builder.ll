; ModuleID = 'functions.c'
source_filename = "functions.c"
target triple = "arm64-apple-darwin27.0.0"

@.str = private unnamed_addr constant [11 x i8] c"value: %d\0A\00", align 1

define i32 @main() {
entry:
  %r = alloca i32, align 4
  %calladdFn = call i32 @add(i32 3, i32 4)
  store i32 %calladdFn, ptr %r, align 4
  %r.show = load i32, ptr %r, align 4
  call void @show(i32 %r.show)
  %r.ret = load i32, ptr %r, align 4
  ret i32 %r.ret
}

define i32 @add(i32 %x, i32 %y) {
entry:
  %x.addr = alloca i32, align 4
  %y.addr = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  store i32 %y, ptr %y.addr, align 4
  %x.val = load i32, ptr %x.addr, align 4
  %y.val = load i32, ptr %y.addr, align 4
  %add = add nsw i32 %x.val, %y.val
  ret i32 %add
}

define void @show(i32 %v) {
entry:
  %v.addr = alloca i32, align 4
  store i32 %v, ptr %v.addr, align 4
  %v.val = load i32, ptr %v.addr, align 4
  %call = call i32 (ptr, ...) @printf(ptr @.str, i32 %v.val)
  ret void
}

declare i32 @printf(ptr, ...)
