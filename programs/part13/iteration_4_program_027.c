/* test_source.c - Simple program to generate gcov data */
#include <stdio.h>

void func1() {
    printf("Function 1 called\n");
}

void func2() {
    printf("Function 2 called\n");
}

int main() {
    int i;
    
    printf("Test program starting\n");
    
    // Call functions multiple times to generate coverage data
    for (i = 0; i < 10; i++) {
        func1();
    }
    
    for (i = 0; i < 5; i++) {
        func2();
    }
    
    printf("Test program completed\n");
    return 0;
}
