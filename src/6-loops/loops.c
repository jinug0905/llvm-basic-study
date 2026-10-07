#include <stdio.h>

int main() {
    int sum = 0;
    for (int i = 1; i <= 10; i++) {
        sum = sum + i;
    }

    int j = 1;
    int prod = 1;
    while (j <= 5) {
        prod = prod * j;
        j = j + 1;
    }
    return sum + prod;
}