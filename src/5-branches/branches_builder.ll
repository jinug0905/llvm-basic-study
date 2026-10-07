; ModuleID = 'branches.c'
source_filename = "branches.c"
target triple = "arm64-apple-darwin27.0.0"

define i32 @main() {
entry:
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  store i32 3, ptr %a, align 4
  store i32 7, ptr %b, align 4
  %result = alloca i32, align 4
  store i32 0, ptr %result, align 4
  %a.val = load i32, ptr %a, align 4
  %b.val = load i32, ptr %b, align 4
  %cmp = icmp sgt i32 %a.val, %b.val
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %a.then = load i32, ptr %a, align 4
  store i32 %a.then, ptr %result, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %b.else = load i32, ptr %b, align 4
  store i32 %b.else, ptr %result, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %level = alloca i32, align 4
  store i32 2, ptr %level, align 4
  %level.val = load i32, ptr %level, align 4
  switch i32 %level.val, label %sw.default [
    i32 1, label %sw.1
    i32 2, label %sw.2
  ]

sw.1:                                             ; preds = %if.end
  %r1 = load i32, ptr %result, align 4
  %r1.add = add nsw i32 %r1, 90
  store i32 %r1.add, ptr %result, align 4
  br label %sw.end

sw.2:                                             ; preds = %if.end
  %r2 = load i32, ptr %result, align 4
  %r2.add = add nsw i32 %r2, 80
  store i32 %r2.add, ptr %result, align 4
  br label %sw.end

sw.default:                                       ; preds = %if.end
  %rd = load i32, ptr %result, align 4
  %rd.add = add nsw i32 %rd, 70
  store i32 %rd.add, ptr %result, align 4
  br label %sw.end

sw.end:                                           ; preds = %sw.default, %sw.2, %sw.1
  %ret.val = load i32, ptr %result, align 4
  ret i32 %ret.val
}
