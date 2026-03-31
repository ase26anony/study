/* Simple test program to generate gcov data */
#include <stdio.h>

void func1() {
    printf("Function 1 called\n");
}

void func2() {
    printf("Function 2 called\n");
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
}

int main() {
    printf("Starting test program\n");
    func1();
    func2();
    printf("Ending test program\n");
    return 0;
}
