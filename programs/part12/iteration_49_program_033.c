/* test_gcov.c - Minimal program to generate GCOV data */
#include <stdio.h>

void helper_function(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else if (x < 0) {
        printf("Negative: %d\n", x);
    } else {
        printf("Zero\n");
    }
}

int main() {
    printf("Generating GCOV data...\n");
    
    // Call function with different values to generate coverage data
    helper_function(1);
    helper_function(-1);
    helper_function(0);
    
    // Loop to generate some arc data
    for (int i = 0; i < 3; i++) {
        printf("Iteration %d\n", i);
    }
    
    return 0;
}
