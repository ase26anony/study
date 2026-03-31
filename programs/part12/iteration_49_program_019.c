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
    
    /* Generate some coverage data */
    for (int i = 0; i < 3; i++) {
        result += helper_function(i);
    }
    
    printf("Result: %d\n", result);
    return 0;
}
