#include <stdio.h>

int main() {
    int a = 3, b = 7;
    int result = 0;

    if (a > b) {
        result = a;
    } else {
        result = b;
    }

    int level = 2;
    switch (level) {
        case 1:
            result = result + 90;
            break;
        case 2:
            result = result + 80;
            break;
        default:
            result = result + 70;
            break;
    }
    return result;
}