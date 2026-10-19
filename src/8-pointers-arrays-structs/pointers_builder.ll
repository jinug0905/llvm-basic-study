; ModuleID = 'pointers.c'
source_filename = "pointers.c"
target triple = "arm64-apple-darwin27.0.0"

%struct.Point = type { i32, i32 }

define i32 @main() {
entry:
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %arr = alloca [4 x i32], align 4
  %p = alloca %struct.Point, align 8
  store i32 1, ptr %a, align 4
  store i32 2, ptr %b, align 4
  call void @swap(ptr %a, ptr %b)
  %arrayidx = getelementptr inbounds [4 x i32], ptr %arr, i64 0, i64 0
  store i32 10, ptr %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds [4 x i32], ptr %arr, i64 0, i64 1
  store i32 20, ptr %arrayidx1, align 4
  %arrayidx2 = getelementptr inbounds [4 x i32], ptr %arr, i64 0, i64 2
  store i32 30, ptr %arrayidx2, align 4
  %arrayidx3 = getelementptr inbounds [4 x i32], ptr %arr, i64 0, i64 3
  store i32 40, ptr %arrayidx3, align 4
  %arr0 = getelementptr inbounds [4 x i32], ptr %arr, i64 0, i64 0
  %arr3 = getelementptr inbounds [4 x i32], ptr %arr, i64 0, i64 3
  call void @swap(ptr %arr0, ptr %arr3)
  %p.x = getelementptr inbounds nuw %struct.Point, ptr %p, i32 0, i32 0
  %p.y = getelementptr inbounds nuw %struct.Point, ptr %p, i32 0, i32 1
  store i32 5, ptr %p.x, align 4
  store i32 6, ptr %p.y, align 4
  call void @swap(ptr %p.x, ptr %p.y)
  %a.v = load i32, ptr %a, align 4
  %b.v = load i32, ptr %b, align 4
  %diff.ab = sub nsw i32 %a.v, %b.v
  %arr0.v = load i32, ptr %arr0, align 4
  %arr3.v = load i32, ptr %arr3, align 4
  %diff.arr = sub nsw i32 %arr0.v, %arr3.v
  %px.v = load i32, ptr %p.x, align 4
  %py.v = load i32, ptr %p.y, align 4
  %diff.p = sub nsw i32 %px.v, %py.v
  %0 = add nsw i32 %diff.ab, %diff.arr
  %ret.val = add nsw i32 %0, %diff.p
  ret i32 %ret.val
}

define void @swap(ptr %a, ptr %b) {
swap.entry:
  %a.addr = alloca ptr, align 8
  %b.addr = alloca ptr, align 8
  %t = alloca i32, align 4
  store ptr %a, ptr %a.addr, align 8
  store ptr %b, ptr %b.addr, align 8
  %a.ptr = load ptr, ptr %a.addr, align 8
  %a.val = load i32, ptr %a.ptr, align 4
  store i32 %a.val, ptr %t, align 4
  %b.ptr = load ptr, ptr %b.addr, align 8
  %b.val = load i32, ptr %b.ptr, align 4
  %a.ptr2 = load ptr, ptr %a.addr, align 8
  store i32 %b.val, ptr %a.ptr2, align 4
  %t.val = load i32, ptr %t, align 4
  %b.ptr2 = load ptr, ptr %b.addr, align 8
  store i32 %t.val, ptr %b.ptr2, align 4
  ret void
}
