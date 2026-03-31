/* Simple test program to generate gcov data */
#include <stdio.h>

void func1() {
    printf("func1 called\n");
}

void func2() {
    printf("func2 called\n");
    for (int i = 0; i < 10; i++) {
        printf("Loop iteration %d\n", i);
    }
}

int main() {
    printf("Test program starting\n");
    func1();
    func2();
    printf("Test program ending\n");
    return 0;
}
