#include <stdio.h>

int add(int x, int y) {
    return x + y;
}

void show(int v) {
    printf("value: %d\n", v);
}

int main() {
    int r = add(3, 4);
    show(r);
    return r;
}
