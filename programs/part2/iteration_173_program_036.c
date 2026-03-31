/* Simple program to generate GCOV data */
#include <stdio.h>

void function1() {
    printf("Function 1 called\n");
}

void function2(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

int main() {
    printf("Starting coverage test program\n");
    
    function1();
    function2(5);
    function2(-3);
    
    printf("Program completed\n");
    return 0;
}
