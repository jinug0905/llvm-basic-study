#include <stdio.h>

int main() {
    int i = 65;
    long l = i;            // sext: i32 -> i64
    short s = (short)i;    // trunc: i32 -> i16
    double d = i;          // sitofp: i32 -> double
    int back = (int)d;     // fptosi: double -> i32
    char c = (char)i;      // trunc: i32 -> i8
    return back;
}