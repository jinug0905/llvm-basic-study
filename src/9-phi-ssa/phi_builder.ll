; ModuleID = 'phi.c'
source_filename = "phi.c"
target triple = "arm64-apple-darwin27.0.0"

define i32 @max(i32 %a, i32 %b) {
entry:
  %cmp = icmp sgt i32 %a, %b
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  br label %cond.end

cond.false:                                       ; preds = %entry
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %a, %cond.true ], [ %b, %cond.false ]
  ret i32 %cond
}

define i32 @both_positive(i32 %a, i32 %b) {
entry:
  %a.pos = icmp sgt i32 %a, 0
  br i1 %a.pos, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %entry
  %b.pos = icmp sgt i32 %b, 0
  br label %land.end

land.end:                                         ; preds = %land.rhs, %entry
  %land = phi i1 [ false, %entry ], [ %b.pos, %land.rhs ]
  %land.ext = zext i1 %land to i32
  ret i32 %land.ext
}

define i32 @sum_to(i32 %n) {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i = phi i32 [ 1, %entry ], [ %i.next, %for.inc ]
  %s = phi i32 [ 0, %entry ], [ %s.next, %for.inc ]
  %cmp = icmp sle i32 %i, %n
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %s.next = add nsw i32 %s, %i
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %i.next = add nsw i32 %i, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret i32 %s
}

define i32 @main() {
entry:
  %call.max = call i32 @max(i32 3, i32 9)
  %call.bothp = call i32 @both_positive(i32 1, i32 2)
  %partial = add nsw i32 %call.max, %call.bothp
  %call.sumto = call i32 @sum_to(i32 10)
  %ret.val = add nsw i32 %partial, %call.sumto
  ret i32 %ret.val
}
