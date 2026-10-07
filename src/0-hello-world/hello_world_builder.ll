; ModuleID = 'hello_world.c'
source_filename = "hello_world.c"
target triple = "arm64-apple-darwin25.6.0"

@.str = private unnamed_addr constant [17 x i8] c"Hello World !!!\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.str)
  ret i32 0
}
