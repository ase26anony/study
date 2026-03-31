/* Simple test program to generate GCOV data */
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
    printf("GCOV Test Program\n");
    
    // Multiple branches for coverage data
    for (int i = -1; i <= 1; i++) {
        helper_function(i);
    }
    
    // Another conditional
    int x = 5;
    if (x > 0) {
        printf("x is positive\n");
    }
    
    return 0;
}
