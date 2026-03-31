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
    printf("Starting test program...\n");
    func1(5);
    func2(3);
    printf("Test program completed.\n");
    return 0;
}
