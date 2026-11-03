int max(int a, int b) {
    return a > b ? a : b;
}

int both_positive(int a, int b) {
    return a > 0 && b > 0;
}

int sum_to(int n) {
    int s = 0;
    for (int i = 1; i <= n; i++) {
        s = s + i;
    }
    return s;
}

int main() {
    return max(3, 9) + both_positive(1, 2) + sum_to(10);
}
