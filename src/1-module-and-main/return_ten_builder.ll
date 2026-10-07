; ModuleID = 'return_ten.c'
source_filename = "return_ten.c"
target triple = "arm64-apple-darwin25.6.0"

define dso_local i32 @main() {
entry:
  ret i32 10
}
