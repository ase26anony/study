/* test_gcov.c - Minimal program to generate GCOV data */
#include <stdio.h>

void helper_function(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

int main() {
    printf("Generating GCOV data...\n");
    
    // Multiple branches for coverage data
    for (int i = -1; i <= 2; i++) {
        helper_function(i);
    }
    
    // Another conditional
    int x = 10;
    if (x > 5) {
        printf("x is greater than 5\n");
    }
    
    return 0;
}
