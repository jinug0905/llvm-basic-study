; ModuleID = 'loops.c'
source_filename = "loops.c"
target triple = "arm64-apple-darwin27.0.0"

define i32 @main() {
entry:
  %sum = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %prod = alloca i32, align 4
  store i32 0, ptr %sum, align 4
  store i32 1, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.cond = load i32, ptr %i, align 4
  %i.cmp = icmp sle i32 %i.cond, 10
  br i1 %i.cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %sum.val = load i32, ptr %sum, align 4
  %i.body = load i32, ptr %i, align 4
  %sum.add = add nsw i32 %sum.val, %i.body
  store i32 %sum.add, ptr %sum, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %i.inc = load i32, ptr %i, align 4
  %i.next = add nsw i32 %i.inc, 1
  store i32 %i.next, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 1, ptr %j, align 4
  store i32 1, ptr %prod, align 4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %for.end
  %j.cond = load i32, ptr %j, align 4
  %j.cmp = icmp sle i32 %j.cond, 5
  br i1 %j.cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %prod.val = load i32, ptr %prod, align 4
  %j.body = load i32, ptr %j, align 4
  %prod.mul = mul nsw i32 %prod.val, %j.body
  store i32 %prod.mul, ptr %prod, align 4
  %j.inc = load i32, ptr %j, align 4
  %j.next = add nsw i32 %j.inc, 1
  store i32 %j.next, ptr %j, align 4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %sum.ret = load i32, ptr %sum, align 4
  %prod.ret = load i32, ptr %prod, align 4
  %ret.val = add nsw i32 %sum.ret, %prod.ret
  ret i32 %ret.val
}
