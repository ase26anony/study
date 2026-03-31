/* test_gcov.c - Minimal program to generate GCOV data */
#include <stdio.h>

int helper(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x - 1;
    }
}

int main() {
    int result = 0;
    
    // Generate some branching for coverage data
    for (int i = 0; i < 3; i++) {
        result += helper(i);
    }
    
    printf("Result: %d\n", result);
    return 0;
}
