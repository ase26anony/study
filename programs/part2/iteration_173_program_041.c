/* Simple test program to generate GCOV data */
#include <stdio.h>

void function1() {
    printf("Function 1\n");
}

void function2(int n) {
    for (int i = 0; i < n; i++) {
        printf("Iteration %d\n", i);
    }
}

int main() {
    printf("Starting test program\n");
    function1();
    function2(3);
    printf("Ending test program\n");
    return 0;
}
