/* Simple test program to generate gcov data */
#include <stdio.h>

void func1() {
    printf("func1 called\n");
}

void func2() {
    printf("func2 called\n");
}

int main() {
    int i;
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            func1();
        } else {
            func2();
        }
    }
    return 0;
}
