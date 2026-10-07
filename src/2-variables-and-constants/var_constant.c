int global_a = 1;

static const int const_arr[4] = {1, 2, 3, 4};

struct Point { int x; int y; };
static const struct Point const_point = {11, 12};

static const char *const_str = "hello";

int main() {
    int local_b = 2;
    local_b = global_a;
    return local_b;
}