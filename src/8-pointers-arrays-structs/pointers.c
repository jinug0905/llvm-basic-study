struct Point { int x; int y; };

void swap(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int main() {
    int a = 1, b = 2;
    swap(&a, &b);

    int arr[4];
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    arr[3] = 40;
    swap(&arr[0], &arr[3]);

    struct Point p;
    p.x = 5;
    p.y = 6;
    swap(&p.x, &p.y);

    return (a - b) + (arr[0] - arr[3]) + (p.x - p.y);
}