/* Simple test program to generate gcov data */
#include <stdio.h>

void func1(int n) {
    for (int i = 0; i < n; i++) {
        printf("func1: %d\n", i);
    }
}

void func2(int n) {
    for (int i = 0; i < n; i++) {
        printf("func2: %d\n", i);
    }
}

int main() {
    printf("Test program for gcov-tool overlap testing\n");
    func1(3);
    func2(5);
    return 0;
}
