/* test.c - Simple test program for gcov-dump */
int main() {
    int x = 0;
    if (x) {
        return 1;
    }
    return 0;
}

int foo(int y) {
    if (y > 0) {
        return y * 2;
    } else {
        return y - 1;
    }
}
