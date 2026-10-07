; ModuleID = 'var_constant.c'
source_filename = "var_constant.c"
target triple = "arm64-apple-darwin25.6.0"

%struct.Point = type { i32, i32 }

@global_a = global i32 1
@const_arr = internal constant [4 x i32] [i32 1, i32 2, i32 3, i32 4]
@const_point = internal constant %struct.Point { i32 11, i32 12 }
@.str = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@const_str = internal constant ptr @.str

define i32 @main() {
entry:
  %local_b = alloca i32, align 4
  store i32 2, ptr %local_b, align 4
  %0 = load i32, ptr @global_a, align 4
  store i32 %0, ptr %local_b, align 4
  %1 = load i32, ptr %local_b, align 4
  ret i32 %1
}
