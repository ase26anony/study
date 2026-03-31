/* test_gcov.c - Minimal C program for GCOV testing */
#include <stdio.h>

int helper_function(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x - 1;
    }
}

int main() {
    int result = 0;
    
    /* Create some branches for coverage data */
    for (int i = 0; i < 3; i++) {
        result += helper_function(i);
    }
    
    /* Another conditional */
    if (result > 5) {
        printf("Result is %d (greater than 5)\n", result);
    } else {
        printf("Result is %d (5 or less)\n", result);
    }
    
    return 0;
}
